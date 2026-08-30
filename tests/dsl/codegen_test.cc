// Code-emission tests for dsl::ToSource (the designer-tool "emit code" half).
//
// ToSource walks a Node tree and prints the equivalent builder expression. The
// structural claims are pinned here as exact fragments (order + indentation);
// callbacks/conditions can never round-trip, so their placeholder forms are
// asserted verbatim too — the contract is that the output stays *compilable*,
// which the placeholder shapes guarantee (no comments inside argument lists).
//
// Note on trailing commas: inner container entries keep their trailing comma
// (legal C++), but the ROOT expression's comma is stripped so the snippet ends
// `...);`.
#include <unigui/dsl/dsl.h>

#include <gtest/gtest.h>
#include <string>

namespace dsl = unigui::dsl;

TEST(DslToSource, NullRoot_ReturnsEmpty) {
    EXPECT_TRUE(dsl::ToSource(nullptr).empty());
}

TEST(DslToSource, LeafLabel_EmitsPreambleAndExpression) {
    const std::string src = dsl::ToSource(dsl::Label("Hello"));
    EXPECT_TRUE(src.starts_with("using namespace unigui::dsl;\n\nNodePtr ui =\n"));
    EXPECT_TRUE(src.find("    Label(\"Hello\");") != std::string::npos);
    EXPECT_TRUE(src.ends_with(";\n"));
}

TEST(DslToSource, WindowSingleChild_CollapsesOntoOneLine) {
    const std::string src = dsl::ToSource(dsl::Window("Demo", dsl::VBox({dsl::Text("Hi")})));
    EXPECT_TRUE(src.find("Window(\"Demo\", VBox({") != std::string::npos);
    EXPECT_TRUE(src.find("      Text(\"Hi\"),") != std::string::npos);
    EXPECT_TRUE(src.find("    })") != std::string::npos); // VBox '}' + Window ')'

    // A leaf child stays fully on one line.
    const std::string leaf = dsl::ToSource(dsl::Window("T", dsl::Label("x")));
    EXPECT_TRUE(leaf.find("Window(\"T\", Label(\"x\"));") != std::string::npos);
}

TEST(DslToSource, Container_ChildrenIndentTwoSpacesPerLevel) {
    const std::string src = dsl::ToSource(dsl::VBox({
        dsl::Separator(),
        dsl::HBox({dsl::Button("A"), dsl::Spacing(), dsl::Button("B")}),
    }));
    EXPECT_TRUE(src.find("    VBox({") != std::string::npos);
    EXPECT_TRUE(src.find("      Separator(),") != std::string::npos);
    EXPECT_TRUE(src.find("      HBox({") != std::string::npos);
    EXPECT_TRUE(src.find("        Button(\"A\"),") != std::string::npos);
    EXPECT_TRUE(src.find("        Spacing(),") != std::string::npos);
    EXPECT_TRUE(src.find("        Button(\"B\"),") != std::string::npos);
    EXPECT_TRUE(src.find("      }),") != std::string::npos);
    EXPECT_TRUE(src.find("    })") != std::string::npos);
}

TEST(DslToSource, Button_DefaultOmitsVariant_PrimaryEmitsIt) {
    const std::string plain = dsl::ToSource(dsl::Button("Save"));
    EXPECT_TRUE(plain.find("Button(\"Save\");") != std::string::npos);

    const std::string primary = dsl::ToSource(dsl::Button("Save", dsl::ButtonVariant::Primary));
    EXPECT_TRUE(primary.find("Button(\"Save\", ButtonVariant::Primary);") != std::string::npos);
}

TEST(DslToSource, SliderFloat_EmitsNumericLiteralsWithFSuffix) {
    const std::string src = dsl::ToSource(dsl::SliderFloat("Gain", 0.0f, 1.5f));
    EXPECT_TRUE(src.find("SliderFloat(\"Gain\", 0.0f, 1.5f);") != std::string::npos);
}

TEST(DslToSource, Flex_EmitsGapJustifyAndWeights) {
    const std::string unweighted = dsl::ToSource(
        dsl::Flex({dsl::Label("a"), dsl::Label("b")}, 8.0f, unigui::layout::FlexJustify::Center));
    EXPECT_TRUE(unweighted.find("Flex({") != std::string::npos);
    EXPECT_TRUE(unweighted.find("    }, 8.0f, FlexJustify::Center);") != std::string::npos);

    const std::string weighted =
        dsl::ToSource(dsl::Flex({dsl::Label("a"), dsl::Label("b")}, {1.0f, 2.0f}, 4.0f,
                                unigui::layout::FlexJustify::SpaceBetween));
    EXPECT_TRUE(weighted.find("    }, {1.0f, 2.0f}, 4.0f, FlexJustify::SpaceBetween);") !=
                std::string::npos);
}

TEST(DslToSource, Bindings_EmitCompilableTrailingNotes) {
    bool flag = false;
    float gain = 0.5f;
    std::string name;
    const std::string src =
        dsl::ToSource(dsl::VBox({dsl::CheckBox("On", &flag), dsl::SliderFloat("Gain", &gain, 0, 1),
                                 dsl::InputText("Name", &name)}));
    EXPECT_TRUE(src.find("CheckBox(\"On\"), // bound to an external bool") != std::string::npos);
    EXPECT_TRUE(src.find("SliderFloat(\"Gain\", 0.0f, 1.0f), // bound to an external float") !=
                std::string::npos);
    EXPECT_TRUE(src.find("InputText(\"Name\"), // bound to an external string") !=
                std::string::npos);
}

TEST(DslToSource, ControlFlow_EmitsCompilablePlaceholders) {
    const std::string ifSrc = dsl::ToSource(dsl::If([] { return true; }, dsl::Text("then")));
    EXPECT_TRUE(ifSrc.find("If([] { return true; } /* condition */,") != std::string::npos);
    EXPECT_TRUE(ifSrc.find("Text(\"then\"),") != std::string::npos);
    EXPECT_TRUE(ifSrc.find("    );") != std::string::npos);

    const std::string ifElseSrc =
        dsl::ToSource(dsl::IfElse([] { return true; }, dsl::Text("yes"), dsl::Text("no")));
    EXPECT_TRUE(ifElseSrc.find("IfElse([] { return true; } /* condition */,") != std::string::npos);
    EXPECT_TRUE(ifElseSrc.find("Text(\"yes\"),") != std::string::npos);
    EXPECT_TRUE(ifElseSrc.find("Text(\"no\"),") != std::string::npos);

    const std::string forSrc =
        dsl::ToSource(dsl::For(3, [](int i) { return dsl::Label(std::to_string(i)); }));
    EXPECT_TRUE(forSrc.find("For(3, [](int i) { /* item builder */ return "
                            "Label(std::to_string(i)); });") != std::string::npos);

    const std::string customSrc = dsl::ToSource(dsl::Custom([] {}));
    EXPECT_TRUE(customSrc.find("Custom([] { /* draw lambda */ });") != std::string::npos);
}

TEST(DslToSource, Escaping_QuotesBackslashesAndNewlines) {
    const std::string src = dsl::ToSource(dsl::Label("a\"b\\c\nd"));
    EXPECT_TRUE(src.find("Label(\"a\\\"b\\\\c\\nd\");") != std::string::npos);
}
