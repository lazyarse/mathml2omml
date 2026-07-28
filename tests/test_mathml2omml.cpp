#include <gtest/gtest.h>
#include <QByteArray>
#include <QXmlStreamWriter>
#include "mathml2omml.h"

static QString ommlFromMathml(const QString &mathmlXml)
{
    QByteArray buf;
    QXmlStreamWriter w(&buf);
    w.setAutoFormatting(false);
    w.writeNamespace(
        QStringLiteral("http://schemas.openxmlformats.org/officeDocument/2006/math"),
        QStringLiteral("m"));
    w.writeStartElement("x");
    if (!MathmlToOmml::convert(mathmlXml, w))
        return {};
    w.writeEndElement();
    w.writeEndDocument();
    return QString::fromUtf8(buf);
}

TEST(Mathml2OmmlTest, SimpleSuperscript)
{
    QString mml =
        "<math xmlns=\"http://www.w3.org/1998/Math/MathML\">"
        "<semantics>"
        "<mrow><msup><mi>x</mi><mn>2</mn></msup></mrow>"
        "<annotation encoding=\"application/x-tex\">x^2</annotation>"
        "</semantics>"
        "</math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:oMath")) << "Must produce m:oMath element";
    EXPECT_TRUE(omml.contains("<m:sSup>")) << "Superscript must produce m:sSup";
    EXPECT_TRUE(omml.contains("<m:e>")) << "Must have m:e for base";
    EXPECT_TRUE(omml.contains("<m:sup>")) << "Must have m:sup for exponent";
    EXPECT_TRUE(omml.contains("<m:t") && omml.contains(">x<")) << "Base text 'x' must be present";
    EXPECT_TRUE(omml.contains("<m:t") && omml.contains(">2<")) << "Exponent text '2' must be present";
    EXPECT_FALSE(omml.contains("x^2")) << "Raw TeX must not appear in OMML output";
}

TEST(Mathml2OmmlTest, Fraction)
{
    QString mml =
        "<math><semantics>"
        "<mrow><mfrac><mi>a</mi><mi>b</mi></mfrac></mrow>"
        "<annotation encoding=\"application/x-tex\">\\frac{a}{b}</annotation>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:f>")) << "Fraction must produce m:f";
    EXPECT_TRUE(omml.contains("<m:num>")) << "Must have m:num for numerator";
    EXPECT_TRUE(omml.contains("<m:den>")) << "Must have m:den for denominator";
    EXPECT_TRUE(omml.contains(">a<")) << "Numerator text 'a' must be present";
    EXPECT_TRUE(omml.contains(">b<")) << "Denominator text 'b' must be present";
}

TEST(Mathml2OmmlTest, SquareRoot)
{
    QString mml =
        "<math><semantics>"
        "<mrow><msqrt><mi>x</mi></msqrt></mrow>"
        "<annotation encoding=\"application/x-tex\">\\sqrt{x}</annotation>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:rad>")) << "Square root must produce m:rad";
    EXPECT_TRUE(omml.contains("<m:deg/>") || omml.contains("<m:deg />")
                || omml.contains("<m:deg></m:deg>"))
        << "m:deg must be empty for sqrt";
    EXPECT_TRUE(omml.contains(">x<")) << "Radicand text 'x' must be present";
}

TEST(Mathml2OmmlTest, NthRootReordersChildren)
{
    QString mml =
        "<math><semantics>"
        "<mrow><mroot><mi>x</mi><mn>3</mn></mroot></mrow>"
        "<annotation encoding=\"application/x-tex\">\\sqrt[3]{x}</annotation>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    int degPos = omml.indexOf("<m:deg>");
    int ePos = omml.indexOf("<m:e>");
    EXPECT_GE(degPos, 0) << "m:deg must be present";
    EXPECT_GE(ePos, 0) << "m:e must be present";
    EXPECT_LT(degPos, ePos) << "m:deg must precede m:e in OMML output";
    EXPECT_TRUE(omml.contains(">3<")) << "Degree value '3' must be present";
    EXPECT_TRUE(omml.contains(">x<")) << "Radicand 'x' must be present";
}

TEST(Mathml2OmmlTest, Subscript)
{
    QString mml =
        "<math><semantics>"
        "<mrow><msub><mi>a</mi><mn>1</mn></msub></mrow>"
        "<annotation encoding=\"application/x-tex\">a_1</annotation>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:sSub>")) << "Subscript must produce m:sSub";
    EXPECT_TRUE(omml.contains("<m:sub>")) << "Must have m:sub element";
    EXPECT_TRUE(omml.contains(">a<")) << "Base 'a' must be present";
    EXPECT_TRUE(omml.contains(">1<")) << "Subscript '1' must be present";
}

TEST(Mathml2OmmlTest, SubSup)
{
    QString mml =
        "<math><semantics>"
        "<mrow><msubsup><mi>A</mi><mn>1</mn><mn>2</mn></msubsup></mrow>"
        "<annotation encoding=\"application/x-tex\">A_1^2</annotation>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:sSubSup>")) << "SubSup must produce m:sSubSup";
    EXPECT_TRUE(omml.contains("<m:sub>")) << "Must have m:sub element";
    EXPECT_TRUE(omml.contains("<m:sup>")) << "Must have m:sup element";
    EXPECT_TRUE(omml.contains(">A<")) << "Base 'A' must be present";
    EXPECT_TRUE(omml.contains(">1<")) << "Subscript '1' must be present";
    EXPECT_TRUE(omml.contains(">2<")) << "Superscript '2' must be present";
}

TEST(Mathml2OmmlTest, ItalicStyle)
{
    QString mml =
        "<math><semantics>"
        "<mrow><mi>x</mi></mrow>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:sty>i</m:sty>")) << "mi must use italic style";
}

TEST(Mathml2OmmlTest, OverAndUnderLimits)
{
    QString mml =
        "<math><semantics>"
        "<mrow><mover><mi>x</mi><mo>&#xAF;</mo></mover></mrow>"
        "</semantics></math>";

    QString omml = ommlFromMathml(mml);

    EXPECT_TRUE(omml.contains("<m:limUpp>")) << "mover must produce m:limUpp";
    EXPECT_TRUE(omml.contains(">x<")) << "Base 'x' must be present";
}
