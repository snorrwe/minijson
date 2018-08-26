# MiniJson

[![Build Status](https://travis-ci.org/snorrwe/minijson.svg?branch=master)](https://travis-ci.org/snorrwe/minijson)
[![codecov](https://codecov.io/gh/snorrwe/minijson/branch/master/graph/badge.svg)](https://codecov.io/gh/snorrwe/minijson)

Lightweight header-only JSON library written in C++17

## Requirements

- C++17 compatible compiler
- CMake 3.8+ (recommended)

## Usage

To be able to parse custom data structures the structures must provide a `static` `constexpr` method named `json_propertied` that returns a tuple of the `public` data members that we wish to serialise / parse.

```cpp
struct Seed
{
    // Serialisable data members must be declared public
    float radius = 0.0;

    constexpr static auto json_properties()
    {
        return std::make_tuple(mini_json::property(&Seed::radius, "radius"));
    }
};
```

## Usage example

```cpp

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

TEST_F(TestJsonParser, CanReadJsonIntoObject)
{
    /*
    {
        "color":"red",
        "size": -25,
        "seed": {
            "radius": -3.14
        }
    }
    */
    const auto json = "{\"color\":\"red\",\"size\": -25,\"seed\":{\"radius\":-3.14}}"s;

    auto result = mini_json::parse<Apple>(json.begin(), json.end());

    EXPECT_EQ(result.color, "red");
    EXPECT_EQ(result.size, -25);
    EXPECT_FLOAT_EQ(result.seed.radius, -3.14f);
}
```

## Supported types:

- Any `T` that implements the `json_properties` static member function
- `std::vector<T>` for any json serialisable `T`
- `std::string`
- `int`
- `size_t`
- `float`
- `double`
