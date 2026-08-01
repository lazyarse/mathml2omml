# mathml2omml

A zero-dependency C++17 library for converting between Mathematical Markup Language (MathML) and Office Mathematical Markup Language (OMML).

## Build

```sh
cmake -B build
cmake --build build
```

Include headers: `#include "mathml2omml.h"` and link `libmathml2omml.a`.

Optional tests (requires Google Test):

```sh
cmake -B build -DBUILD_TESTS=ON
cmake --build build
./build/tests/test_mathml2omml
```

## API

### MathML → OMML

```cpp
#include "mathml2omml.h"
#include <iostream>

int main() {
    std::string mml =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">"
        "<msup><mi>x</mi><mn>2</mn></msup>"
        "</math>";

    std::string omml = MathmlToOmml::convert(mml);
    std::cout << omml << std::endl;
    // <m:oMath xmlns:m="http://schemas.openxmlformats.org/officeDocument/2006/math">
    //   <m:sSup><m:e><m:r><m:t>x</m:t></m:r></m:e>
    //   <m:sup><m:r><m:t>2</m:t></m:r></m:sup></m:sSup>
    // </m:oMath>
}
```

### OMML → MathML

```cpp
#include "mathml2omml.h"
#include <iostream>

int main() {
    std::string omml =
        "<m:oMath xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\">"
        "<m:f>"
        "<m:num><m:r><m:t>a</m:t></m:r></m:num>"
        "<m:den><m:r><m:t>b</m:t></m:r></m:den>"
        "</m:f>"
        "</m:oMath>";

    std::string mml = OmmlToMathml::convert(omml);
    std::cout << mml << std::endl;
    // <math><mfrac><mi>a</mi><mi>b</mi></mfrac></math>
}
```

### Stream-oriented conversion

Both converters accept a custom sink for streaming output:

```cpp
#include <string>
#include <vector>

// A custom XmlSink that pretty-prints XML output with indentation.
// Useful for debugging or generating human-readable OMML.
struct PrettySink : XmlSink {
    std::vector<std::string> stack_;  // tracks open elements for end-tag names + indentation
    std::string out_;
    bool openTag_ = false;

    void startElement(const std::string &name) override {
        closeOpenTag();
        out_ += std::string(stack_.size() * 2, ' ') + '<' + name;
        openTag_ = true;
        stack_.push_back(name);
    }

    void endElement() override {
        if (openTag_) {
            closeVoidTag();
            stack_.pop_back();
        } else {
            out_ += std::string((stack_.size() - 1) * 2, ' ')
                  + "</" + stack_.back() + ">\n";
            stack_.pop_back();
        }
    }

    void attribute(const std::string &name, const std::string &value) override {
        out_ += ' ' + name + "=\"" + value + '"';
    }

    void characters(const std::string &text) override {
        closeOpenTag();
        out_ += std::string(stack_.size() * 2, ' ') + text + '\n';
    }

    std::string result() const { return out_; }

private:
    void closeOpenTag() {
        if (openTag_) { out_ += ">\n"; openTag_ = false; }
    }

    void closeVoidTag() {
        if (openTag_) { out_ += "/>\n"; openTag_ = false; }
    }
};

PrettySink sink;
MathmlToOmml::convert(mathmlInput, sink);  // or OmmlToMathml::convert(ommlInput, sink);
```

## OMML elements supported in reverse (OMML → MathML)

| OMML | MathML |
|---|---|
| `m:oMath` | `<math display="inline">` |
| `m:oMathPara` | `<math display="block">` |
| `m:r` (with `m:t`) | `<mi>` / `<mn>` / `<mo>` / `<mtext>` (content heuristic + `m:nor` flag) |
| `m:f` | `<mfrac>` (supports `noBar` → linethickness 0, `skw` → bevelled) |
| `m:sSup` / `m:sSub` / `m:sSubSup` | `<msup>` / `<msub>` / `<msubsup>` |
| `m:sPre` | `<mmultiscripts>` + `<mprescripts>` |
| `m:rad` | `<msqrt>` or `<mroot>` (detects empty `deg`; `degHide` → sqrt) |
| `m:limUpp` / `m:limLow` | `<mover>` / `<munder>` |
| `m:nary` | `<mo>` + `<msubsup>` / `<munderover>` (based on `limLoc`) |
| `m:d` | `<mrow>` + `<mo>` delimiters |
| `m:acc` | `<mover>` with combining accent |
| `m:bar` | `<mover>` / `<munder>` with overline |
| `m:func` | `<mrow>` + `<mi>` + U+2061 |
| `m:groupChr` | `<mover>` / `<munder>` with brace char |
| `m:m` / `m:mr` / `m:mc` | `<mtable>` / `<mtr>` / `<mtd>` |
| `m:eqArr` | `<mtable>` |
| `m:phant` | `<mphantom>` |
| `m:borderBox` | `<menclose notation="box">` |
| `m:box` | transparent passthrough |
| `m:br` | `<mspace linebreak="newline"/>` |

Font properties (`m:sty`, `m:scr`, `m:nor`) are mapped to the MathML `mathvariant` attribute.

## MathML elements supported in forward (MathML → OMML)

| MathML | OMML |
|---|---|
| `mi`, `mn`, `mo`, `mtext`, `ms` | `<m:r>` (with `m:sty` for italic/bold) |
| `msup` / `msub` / `msubsup` | `<m:sSup>` / `<m:sSub>` / `<m:sSubSup>` |
| `mfrac` | `<m:f>` |
| `msqrt` / `mroot` | `<m:rad>` (empty `deg` for sqrt) |
| `mover` / `munder` | `<m:limUpp>` / `<m:limLow>` |
| `munderover` | `<m:limLow>` nested with `<m:limUpp>` |
| `mtable` / `mtr` / `mtd` | `<m:m>` / `<m:mr>` / `<m:mc>` |
| `mstyle`, `mpadded`, `merror`, `menclose` | transparent passthrough |
| `annotation`, `annotation-xml` | dropped |

## Dependencies

None. C++17 standard library only. Google Test (optional, for tests).
