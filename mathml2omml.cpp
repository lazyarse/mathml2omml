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

    // Move to the first start element, skipping XML declaration / comments / whitespace
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

// ── OMML output builder ──────────────────────────────────────────────────────

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

static std::string tag(const std::string &name, const std::string &content) {
    return "<" + name + ">" + content + "</" + name + ">";
}

static std::string tagA(const std::string &name, const std::string &attr,
                         const std::string &val, const std::string &content) {
    return "<" + name + " " + attr + "=\"" + val + "\">" + content + "</" + name + ">";
}

static std::string emptyTag(const std::string &name) {
    return "<" + name + "/>";
}

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

// ── Recursive OMML emitter ────────────────────────────────────────────────────

using MmlNode = XmlReader::MmlNode;

static std::string emitTextRun(const MmlNode &node) {
    std::string style = defaultStyleForTag(node.tag);
    auto it = node.attrs.find("mathvariant");
    if (it != node.attrs.end()) {
        std::string s = mathvariantToStyle(it->second);
        if (!s.empty()) style = s;
    }

    std::string rpr;
    if (style != "p")
        rpr = tag("m:rPr", tag("m:sty", style));

    std::string t = tagA("m:t", "xml:space", "preserve", escXml(node.text));
    return tag("m:r", rpr + t);
}

static std::string emitOmml(const MmlNode &node) {
    const std::string &t = node.tag;

    if (t == "math" || t == "semantics") {
        std::string r;
        for (const auto &c : node.children) r += emitOmml(c);
        return r;
    }
    if (t == "annotation" || t == "annotation-xml") return {};

    if (t == "mi" || t == "mn" || t == "mo" || t == "mtext" || t == "ms")
        return emitTextRun(node);
    if (t == "mspace") return {};

    if (t == "mrow") {
        std::string r;
        for (const auto &c : node.children) r += emitOmml(c);
        return r;
    }

    if (t == "msup") {
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string sup = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        return tag("m:sSup", tag("m:e", base) + tag("m:sup", sup));
    }

    if (t == "msub") {
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string sub = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        return tag("m:sSub", tag("m:e", base) + tag("m:sub", sub));
    }

    if (t == "msubsup") {
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string sub = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        std::string sup = node.children.size() >= 3 ? emitOmml(node.children[2]) : std::string{};
        return tag("m:sSubSup", tag("m:e", base) + tag("m:sub", sub) + tag("m:sup", sup));
    }

    if (t == "mfrac") {
        std::string num = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string den = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        return tag("m:f", tag("m:num", num) + tag("m:den", den));
    }

    if (t == "msqrt") {
        std::string content;
        for (const auto &c : node.children) content += emitOmml(c);
        return tag("m:rad", emptyTag("m:deg") + tag("m:e", content));
    }

    if (t == "mroot") {
        // MathML children: [radicand, degree]; OMML: deg first, then e
        std::string deg = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        return tag("m:rad", tag("m:deg", deg) + tag("m:e", base));
    }

    if (t == "mover") {
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string over = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        return tag("m:limUpp", tag("m:e", base) + tag("m:lim", over));
    }

    if (t == "munder") {
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string under = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        return tag("m:limLow", tag("m:e", base) + tag("m:lim", under));
    }

    if (t == "munderover") {
        std::string base = node.children.size() >= 1 ? emitOmml(node.children[0]) : std::string{};
        std::string under = node.children.size() >= 2 ? emitOmml(node.children[1]) : std::string{};
        std::string over = node.children.size() >= 3 ? emitOmml(node.children[2]) : std::string{};
        return tag("m:limLow",
                   tag("m:e", tag("m:limUpp", tag("m:e", base) + tag("m:lim", over)))
                   + tag("m:lim", under));
    }

    if (t == "mtable") {
        std::string r;
        for (const auto &c : node.children) r += emitOmml(c);
        return tag("m:m", r);
    }

    if (t == "mtr") {
        std::string r;
        for (const auto &c : node.children) r += emitOmml(c);
        return tag("m:mr", r);
    }

    if (t == "mtd") {
        std::string r;
        for (const auto &c : node.children) r += emitOmml(c);
        return tag("m:mc", r);
    }

    if (t == "mstyle" || t == "mpadded" || t == "merror" || t == "menclose") {
        std::string r;
        for (const auto &c : node.children) r += emitOmml(c);
        return r;
    }

    // Unknown — process children
    std::string r;
    for (const auto &c : node.children) r += emitOmml(c);
    return r;
}

// ── Public API ────────────────────────────────────────────────────────────────

std::string MathmlToOmml::convert(const std::string &mathmlXml)
{
    if (mathmlXml.empty()) return {};

    XmlReader reader(mathmlXml);
    if (!reader.seekToFirstElement()) return {};

    MmlNode root = reader.readElement();

    const char *ns = "xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\"";
    return "<m:oMath " + std::string(ns) + ">" + emitOmml(root) + "</m:oMath>";
}
