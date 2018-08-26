#include "json.h"
#include "gtest/gtest.h"
#include <iostream>
#include <sstream>
#include <string>

using namespace mini_json;
using namespace std::string_literals;

namespace {
struct Apple {
    std::string color = "";
    int size = 0;

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&Apple::color, "color"),
            mini_json::property(&Apple::size, "size"));
    }

    bool operator==(Apple const& t) const { return color == t.color && size == t.size; }
    bool operator!=(Apple const& t) const { return !operator==(t); }
};

struct AppleTree {
    std::string id = "";
    std::vector<Apple> apples = {};

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&AppleTree::apples, "apples"),
            mini_json::property(&AppleTree::id, "id"));
    }

    bool operator==(AppleTree const& t) const
    {
        return id == t.id && [&]() {
            auto j = t.apples.begin();
            for (auto i = apples.begin(); i != apples.end() && j != t.apples.end(); ++i, ++j) {
                if (*i != *j)
                    return false;
            }
            return true;
        }();
    }

    bool operator!=(AppleTree const& t) const { return !operator==(t); }
};

class TestJsonSerializer : public ::testing::Test {
    void SetUp()
    {
        tree.id = "tree_id"s;
        tree.apples = { Apple { "red"s, 0 }, Apple { "blue"s, 1 }, Apple { "green"s, 2 } };
    }

public:
    AppleTree tree;
};

TEST_F(TestJsonSerializer, DoesSerialize)
{
    auto result = std::stringstream();
    mini_json::serialize(tree, result);
    EXPECT_FALSE(result.str().empty());
}

TEST_F(TestJsonSerializer, ParsedSerializationEqualsOriginal)
{
    auto json = std::stringstream();
    mini_json::serialize(tree, json);

    auto jsonStr = json.str();
    auto result = mini_json::parse<AppleTree>(jsonStr.begin(), jsonStr.end());
    EXPECT_EQ(result, tree);
}
}

