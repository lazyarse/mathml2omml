#pragma once

#include <string>

struct XmlSink {
    virtual ~XmlSink() = default;
    virtual void startElement(const std::string &qualifiedName) = 0;
    virtual void endElement() = 0;
    virtual void attribute(const std::string &qualifiedName,
                           const std::string &value) = 0;
    virtual void characters(const std::string &text) = 0;
};

struct MathmlToOmml {
    static bool convert(const std::string &mathmlXml, XmlSink &sink);
    static std::string convert(const std::string &mathmlXml);
};

struct OmmlToMathml {
    static bool convert(const std::string &ommlXml, XmlSink &sink);
    static std::string convert(const std::string &ommlXml);
};
