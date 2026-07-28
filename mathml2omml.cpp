#include "mathml2omml.h"

#include <map>
#include <vector>
#include <cstdlib>
#include <algorithm>

// ── Simple XML reader (MathML subset) ─────────────────────────────────────────

class XmlReader {
    const std::string &s;
    size_t p = 0;

    char peek() const { return p < s.size() ? s[p] : '\0'; }
    char next() { return p < s.size() ? s[p++] : '\0'; }

    void skipWs() {
        while (p < s.size() && (s[p] == ' ' || s[p] == '\n' || s[p] == '\t' || s[p] == '\r'))
            ++p;
    }

    std::string readName() {
        size_t start = p;
        if (p < s.size() && (isAlpha(s[p]) || s[p] == '_' || s[p] == ':')) {
            ++p;
            while (p < s.size() && (isAlphaNum(s[p]) || s[p] == '_' || s[p] == '-' || s[p] == ':'))
                ++p;
        }
        return s.substr(start, p - start);
    }

    std::string readAttrValue() {
        if (p >= s.size()) return {};
        char q = s[p];
        if (q != '"' && q != '\'') return {};
        ++p;
        size_t start = p;
        while (p < s.size() && s[p] != q) ++p;
        std::string raw = s.substr(start, p - start);
        if (p < s.size()) ++p;
        return decodeEnt(raw);
    }

    std::string decodeEnt(const std::string &raw) {
        std::string out;
        out.reserve(raw.size());
        for (size_t i = 0; i < raw.size(); ++i) {
            if (raw[i] == '&') {
                size_t semi = raw.find(';', i);
                if (semi == std::string::npos) { out += raw[i]; continue; }
                std::string ent = raw.substr(i + 1, semi - i - 1);
                if (ent == "amp") out += '&';
                else if (ent == "lt") out += '<';
                else if (ent == "gt") out += '>';
                else if (ent == "quot") out += '"';
                else if (ent == "apos") out += '\'';
                else if (ent.size() > 1 && ent[0] == '#') {
                    bool hex = ent.size() > 2 && ent[1] == 'x';
                    const char *nptr = ent.c_str() + (hex ? 2 : 1);
                    char *end = nullptr;
                    long cp = std::strtol(nptr, &end, hex ? 16 : 10);
                    if (end != nptr && cp > 0 && cp <= 0x10FFFF)
                        out += static_cast<char>(cp);
                    else
                        out += raw.substr(i, semi - i + 1);
                } else {
                    out += raw.substr(i, semi - i + 1);
                }
                i = semi;
            } else {
                out += raw[i];
            }
        }
        return out;
    }

    static bool isAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); }
    static bool isAlphaNum(char c) { return isAlpha(c) || (c >= '0' && c <= '9'); }

public:
    explicit XmlReader(const std::string &str) : s(str) {}

    struct MmlNode {
        std::string tag;
        std::map<std::string, std::string> attrs;
        std::string text;
        std::vector<MmlNode> children;
    };

    bool seekToFirstElement() {
        while (p < s.size()) {
            skipWs();
            if (p >= s.size()) return false;
            if (s[p] != '<') return false;
            if (p + 1 < s.size() && s[p + 1] == '?') {
                size_t end = s.find("?>", p);
                if (end == std::string::npos) return false;
                p = end + 2;
                continue;
            }
            if (p + 3 < s.size() && s[p + 1] == '!' && s[p + 2] == '-' && s[p + 3] == '-') {
                size_t end = s.find("-->", p);
                if (end == std::string::npos) return false;
                p = end + 3;
                continue;
            }
            return true;
        }
        return false;
    }

    MmlNode readElement() {
        MmlNode node;
        if (p >= s.size() || s[p] != '<') return node;
        ++p;

        if (p < s.size() && s[p] == '/') {
            ++p;
            readName();
            skipWs();
            if (p < s.size() && s[p] == '>') ++p;
            return node;
        }

        node.tag = readName();
        skipWs();
        while (p < s.size() && s[p] != '>' && s[p] != '/') {
            std::string an = readName();
            skipWs();
            std::string av;
            if (p < s.size() && s[p] == '=') {
                ++p; skipWs();
                av = readAttrValue();
            }
            if (!an.empty()) node.attrs[an] = av;
            skipWs();
        }

        bool selfClose = false;
        if (p < s.size() && s[p] == '/') { selfClose = true; ++p; }
        if (p < s.size() && s[p] == '>') ++p;

        if (selfClose) return node;

        while (p < s.size()) {
            skipWs();
            if (p >= s.size()) break;
            if (s[p] == '<') {
                if (p + 1 < s.size() && s[p + 1] == '/') {
                    p += 2;
                    std::string en = readName();
                    skipWs();
                    if (p < s.size() && s[p] == '>') ++p;
                    if (en == node.tag) break;
                } else {
                    node.children.push_back(readElement());
                }
            } else {
                size_t start = p;
                while (p < s.size() && s[p] != '<') ++p;
                std::string txt = s.substr(start, p - start);
                if (!txt.empty())
                    node.text += decodeEnt(txt);
            }
        }
        return node;
    }
};

// ── Style helpers ─────────────────────────────────────────────────────────────

static std::string defaultStyleForTag(const std::string &tag) {
    return tag == "mi" ? "i" : "p";
}

static std::string mathvariantToStyle(const std::string &variant) {
    if (variant == "normal") return "p";
    if (variant == "italic") return "i";
    if (variant == "bold") return "b";
    if (variant == "bold-italic") return "bi";
    if (variant == "double-struck") return "d";
    if (variant == "fraktur") return "f";
    if (variant == "script") return "s";
    return {};
}

// ── Recursive OMML emitter (sink-based) ──────────────────────────────────────

using MmlNode = XmlReader::MmlNode;

static void emitTextRun(const MmlNode &node, OmmlSink &sink) {
    std::string style = defaultStyleForTag(node.tag);
    auto it = node.attrs.find("mathvariant");
    if (it != node.attrs.end()) {
        std::string s = mathvariantToStyle(it->second);
        if (!s.empty()) style = s;
    }

    sink.startElement("m:r");
    if (style != "p") {
        sink.startElement("m:rPr");
        sink.startElement("m:sty");
        sink.attribute("m:val", style);
        sink.endElement(); // m:sty
        sink.endElement(); // m:rPr
    }
    sink.startElement("m:t");
    sink.attribute("xml:space", "preserve");
    sink.characters(node.text);
    sink.endElement(); // m:t
    sink.endElement(); // m:r
}

static void emitOmml(const MmlNode &node, OmmlSink &sink) {
    const std::string &t = node.tag;

    if (t == "math" || t == "semantics") {
        for (const auto &c : node.children) emitOmml(c, sink);
        return;
    }
    if (t == "annotation" || t == "annotation-xml") return;

    if (t == "mi" || t == "mn" || t == "mo" || t == "mtext" || t == "ms")
        return emitTextRun(node, sink);
    if (t == "mspace") return;

    if (t == "mrow") {
        for (const auto &c : node.children) emitOmml(c, sink);
        return;
    }

    if (t == "msup") {
        sink.startElement("m:sSup");
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:sup");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "msub") {
        sink.startElement("m:sSub");
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:sub");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "msubsup") {
        sink.startElement("m:sSubSup");
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:sub");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.startElement("m:sup");
        if (node.children.size() >= 3) emitOmml(node.children[2], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "mfrac") {
        sink.startElement("m:f");
        sink.startElement("m:num");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:den");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "msqrt") {
        sink.startElement("m:rad");
        sink.startElement("m:deg");
        sink.endElement();
        sink.startElement("m:e");
        for (const auto &c : node.children) emitOmml(c, sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "mroot") {
        sink.startElement("m:rad");
        sink.startElement("m:deg");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "mover") {
        sink.startElement("m:limUpp");
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:lim");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "munder") {
        sink.startElement("m:limLow");
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:lim");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "munderover") {
        sink.startElement("m:limLow");
        sink.startElement("m:e");
        sink.startElement("m:limUpp");
        sink.startElement("m:e");
        if (node.children.size() >= 1) emitOmml(node.children[0], sink);
        sink.endElement();
        sink.startElement("m:lim");
        if (node.children.size() >= 3) emitOmml(node.children[2], sink);
        sink.endElement();
        sink.endElement();
        sink.endElement();
        sink.startElement("m:lim");
        if (node.children.size() >= 2) emitOmml(node.children[1], sink);
        sink.endElement();
        sink.endElement();
        return;
    }

    if (t == "mtable") {
        sink.startElement("m:m");
        for (const auto &c : node.children) emitOmml(c, sink);
        sink.endElement();
        return;
    }

    if (t == "mtr") {
        sink.startElement("m:mr");
        for (const auto &c : node.children) emitOmml(c, sink);
        sink.endElement();
        return;
    }

    if (t == "mtd") {
        sink.startElement("m:mc");
        for (const auto &c : node.children) emitOmml(c, sink);
        sink.endElement();
        return;
    }

    if (t == "mstyle" || t == "mpadded" || t == "merror" || t == "menclose") {
        for (const auto &c : node.children) emitOmml(c, sink);
        return;
    }

    // Unknown — process children
    for (const auto &c : node.children) emitOmml(c, sink);
}

// ── String-building sink (keeps string overload working) ──────────────────────

class StringSink : public OmmlSink {
    std::string out_;
    bool pendingAttr_ = false;
    std::vector<std::string> stack_;

    static std::string escXml(const std::string &s) {
        std::string r;
        r.reserve(s.size());
        for (char c : s) {
            switch (c) {
                case '&': r += "&amp;"; break;
                case '<': r += "&lt;"; break;
                case '>': r += "&gt;"; break;
                default: r += c;
            }
        }
        return r;
    }

public:
    void startElement(const std::string &name) override {
        if (pendingAttr_) {
            out_ += '>';
            pendingAttr_ = false;
        }
        out_ += '<';
        out_ += name;
        pendingAttr_ = true;
        stack_.push_back(name);
    }

    void endElement() override {
        if (pendingAttr_) {
            out_ += '/';
            out_ += '>';
            pendingAttr_ = false;
            stack_.pop_back();
            return;
        }
        out_ += "</";
        out_ += stack_.back();
        out_ += '>';
        stack_.pop_back();
    }

    void attribute(const std::string &name, const std::string &value) override {
        out_ += ' ';
        out_ += name;
        out_ += "=\"";
        out_ += escXml(value);
        out_ += '"';
    }

    void characters(const std::string &text) override {
        if (pendingAttr_) {
            out_ += '>';
            pendingAttr_ = false;
        }
        out_ += escXml(text);
    }

    std::string result() const { return out_; }
};

// ── Public API ────────────────────────────────────────────────────────────────

bool MathmlToOmml::convert(const std::string &mathmlXml, OmmlSink &sink)
{
    if (mathmlXml.empty()) return false;

    XmlReader reader(mathmlXml);
    if (!reader.seekToFirstElement()) return false;

    MmlNode root = reader.readElement();

    sink.startElement("m:oMath");
    sink.attribute("xmlns:m",
        "http://schemas.openxmlformats.org/officeDocument/2006/math");
    emitOmml(root, sink);
    sink.endElement();
    return true;
}

std::string MathmlToOmml::convert(const std::string &mathmlXml)
{
    StringSink sink;
    if (convert(mathmlXml, sink))
        return sink.result();
    return {};
}
