#include "json.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace mini_json;
using namespace std::string_literals;

namespace
{
TEST(TestJsonSetter, TestStrEqual)
{
    auto phrase1 = "red";
    auto phrase2 = "red";
    auto phrase3 = "blue";
    auto phrase4 = "red123";
    auto phrase5 = "re";

    EXPECT_TRUE(mini_json::_private::str_equal(phrase1, phrase2));
    EXPECT_FALSE(mini_json::_private::str_equal(phrase1, phrase3));
    EXPECT_FALSE(mini_json::_private::str_equal(phrase1, phrase4));
    EXPECT_FALSE(mini_json::_private::str_equal(phrase1, phrase5));
}

struct Seed
{
    float radius = 0.0;

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&Seed::radius, "radius"));
    }
};

struct Apple
{
    std::string color = "";
    int size = 0;
    Seed seed;

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&Apple::color, "color"),
                               mini_json::property(&Apple::seed, "seed"),
                               mini_json::property(&Apple::size, "size"));
    }
};

class TestJsonParser : public ::testing::Test
{
protected:
};

TEST_F(TestJsonParser, CanReadJsonIntoObject)
{
    const auto json
        = R"a(
            {
                "color":"red",
                "size": -25,
                "seed": {
                    "radius": -3.14
                }
            })a"s;

    auto result = mini_json::parse<Apple>(json.begin(), json.end());

    EXPECT_EQ(result.color, "red");
    EXPECT_EQ(result.size, -25);
    EXPECT_FLOAT_EQ(result.seed.radius, -3.14f);
}

TEST_F(TestJsonParser, CanReadQuotes)
{
    const auto json = R"a({"color":"\"red\"", })a"s;

    auto result = mini_json::parse<Apple>(json.begin(), json.end());

    EXPECT_EQ(result.color, "\"red\"");
}

TEST_F(TestJsonParser, ThrowsParseErrorOnInvalidJson)
{
    auto json = R"a({asd "color": "red","size": -25})a"s;
    EXPECT_THROW(mini_json::parse<Apple>(json.begin(), json.end()), mini_json::ParseError);

    json = R"a({"color"asd: "red","size": -25})a"s;
    EXPECT_THROW(mini_json::parse<Apple>(json.begin(), json.end()), mini_json::ParseError);

    json = R"a({"color"asd: "red","size": -2asd5})a"s;
    EXPECT_THROW(mini_json::parse<Apple>(json.begin(), json.end()), mini_json::ParseError);
}

struct AppleTree
{
    std::string id = "";
    std::vector<Apple> apples = {};

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&AppleTree::apples, "apples"),
                               mini_json::property(&AppleTree::id, "id"));
    }
};

TEST_F(TestJsonParser, CanReadObjectWithVectorOfObjects)
{
    const auto json = R"a({
        "apples": [
              {"color":"red","size":0,"seed":{"radius":0}},
              {"color":"red","size":1,"seed":{"radius":1}},
              {"color":"red","size":2,"seed":{"radius":2}},
              {"color":"red","size":3,"seed":{"radius":3}},
              {"color":"red","size":4,"seed":{"radius":4}}
        ]
    })a"s;

    auto result = mini_json::parse<AppleTree>(json.begin(), json.end());

    ASSERT_EQ(result.apples.size(), 5u);

    float i = 0.0;
    for (auto& apple : result.apples)
    {
        EXPECT_EQ(apple.color, "red");
        EXPECT_EQ(apple.size, i);
        EXPECT_FLOAT_EQ(apple.seed.radius, i);
        ++i;
    }
}

struct Orchid
{
    std::vector<AppleTree> trees = {};

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&Orchid::trees, "trees"));
    }
};

TEST_F(TestJsonParser, CanReadVectorOfObjectsWithVectors)
{
    const auto json = R"a({
                      "trees":[
                        {"id":"tree1","apples": [
                            {"color":"red","size":0,"seed":{"radius":0}},
                            {"color":"red","size":1,"seed":{"radius":1}}
                        ]},
                        {"id":"tree2","apples": [
                            {"color":"red","size":0,"seed":{"radius":0}},
                            {"color":"red","size":1,"seed":{"radius":1}}
                        ]}
                       ]
                      })a"s;

    auto result = mini_json::parse<Orchid>(json.begin(), json.end());

    EXPECT_EQ(result.trees.size(), 2u);
}

TEST_F(TestJsonParser, RaisesExceptionIfNonExistentPropertyIsRead)
{
    auto json = R"a({"color": "red", "size": -25, "fakeproperty": "asd"})a"s;
    EXPECT_THROW(mini_json::parse<Apple>(json.begin(), json.end()), mini_json::UnexpectedPropertyName);
}

TEST_F(TestJsonParser, CanParseStreams)
{
    const auto json = "{\"apples\": ["
                      "{\"color\":\"red\",\"size\":0,\"seed\":{\"radius\":0}},"
                      "{\"color\":\"red\",\"size\":1,\"seed\":{\"radius\":1}},"
                      "{\"color\":\"red\",\"size\":2,\"seed\":{\"radius\":2}},"
                      "{\"color\":\"red\",\"size\":3,\"seed\":{\"radius\":3}},"
                      "{\"color\":\"red\",\"size\":4,\"seed\":{\"radius\":4}}"
                      "]}"s;

    auto stream = std::stringstream();
    stream << json;

    auto result = mini_json::parse<AppleTree>(stream);

    ASSERT_EQ(result.apples.size(), 5u);

    float i = 0;
    for (auto& apple : result.apples)
    {
        EXPECT_EQ(apple.color, "red");
        EXPECT_EQ(apple.size, i);
        EXPECT_FLOAT_EQ(apple.seed.radius, i);
        ++i;
    }
}

TEST_F(TestJsonParser, RaisesExceptionIfCommaIsMissingInList)
{
    struct Simple
    {
        std::vector<int> ns;

        constexpr static auto json_properties()
        {
            return std::make_tuple(mini_json::property(&Simple::ns, "numbers"));
        }
    };
    const auto json = "{"
                      "\"numbers\":[1 2 3 4]"
                      "}"s;
    EXPECT_THROW(mini_json::parse<Simple>(json.begin(), json.end()), mini_json::ParseError);
}


TEST_F(TestJsonParser, AcceptsSpecialCharactersInPropertyNames)
{
    struct Simple
    {
        int ns;

        constexpr static auto json_properties()
        {
            return std::make_tuple(mini_json::property(&Simple::ns, "n:s"));
        }
    };

    const auto json = R"({
                      "n:s": 42
                      })"s;

    const auto simple = mini_json::parse<Simple>(json.begin(), json.end());
    EXPECT_EQ(simple.ns, 42);
}

TEST_F(TestJsonParser, RaisesExceptionIfCommaIsMissingInDict)
{
    struct Simple
    {
        int x = 0;
        int y = 0;

        constexpr static auto json_properties()
        {
            return std::make_tuple(mini_json::property(&Simple::x, "x"),
                                   mini_json::property(&Simple::y, "y"));
        }
    };
    const auto json = "{\"x\": 1 \"y\": 2}"s;
    EXPECT_THROW(mini_json::parse<Simple>(json.begin(), json.end()), mini_json::ParseError);
}
}
