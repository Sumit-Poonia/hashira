// Build notes:
// - Requires C++17 (for <filesystem> on many platforms) and nlohmann/json (single-header).
//   Get nlohmann/json: https://github.com/nlohmann/json (include "nlohmann/json.hpp").
// - For Base64, this example uses a small header-only base64 helper (tobiaslocker/base64).
//   Get "base64.hpp" from https://github.com/tobiaslocker/base64.
// Compile example (Linux/Mac):
//   g++ -std=c++17 main.cpp -o run
// Run:
//   ./run

#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include "nlohmann/json.hpp"   // nlohmann/json single-header
#include "base64.hpp"          // tobiaslocker/base64 header-only

int main() {
    using nlohmann::json;

    // 1) Create JSON with polynomial and Base64-encoded roots.
    // Choose a quadratic with a known constant derivable from the roots.
    // Let a = 2, b = -7, unknown c. Pick roots alpha=2 and beta=5.
    // Then c = a*(alpha*beta) = 2*(10) = 20.
    int a = 2;
    int b = -7;
    // c is unknown initially; will compute from roots using identities.
    std::string alpha_plain = "2";
    std::string beta_plain  = "5";

    std::string alpha_b64 = base64::to_base64(alpha_plain);
    std::string beta_b64  = base64::to_base64(beta_plain);

    json data = {
        {"polynomial", {
            {"a", a},
            {"b", b},
            {"c", nullptr},
            {"form", "ax^2 + bx + c = 0"}
        }},
        {"roots_base64", {
            {"alpha", alpha_b64},
            {"beta",  beta_b64}
        }}
    };

    // Write JSON to file
    const std::string json_path = "polynomial.json";
    {
        std::ofstream out(json_path);
        out << std::setw(2) << data << std::endl;
    }
    std::cout << "JSON written to " << json_path << "\n";

    // 2) Read JSON back
    json loaded;
    {
        std::ifstream in(json_path);
        loaded = json::parse(in);
    }

    // Extract a, b
    int a_loaded = loaded["polynomial"]["a"].get<int>();
    int b_loaded = loaded["polynomial"]["b"].get<int>();

    // 3) Decode Base64 roots and parse to double
    std::string alpha_b64_in = loaded["roots_base64"]["alpha"].get<std::string>();
    std::string beta_b64_in  = loaded["roots_base64"]["beta"].get<std::string>();

    std::string alpha_str = base64::from_base64(alpha_b64_in);
    std::string beta_str  = base64::from_base64(beta_b64_in);

    double alpha = std::stod(alpha_str);
    double beta  = std::stod(beta_str);

    std::cout << "Decoded polynomial and roots:\n";
    std::cout << "  Form: " << loaded["polynomial"]["form"].get<std::string>() << "\n";
    std::cout << "  a = " << a_loaded << ", b = " << b_loaded
              << ", c = " << (loaded["polynomial"]["c"].is_null() ? std::string("null") : loaded["polynomial"]["c"].dump()) << "\n";
    std::cout << "  alpha (root 1) = " << alpha << "\n";
    std::cout << "  beta  (root 2) = " << beta  << "\n";

    // 4) Compute c using product-of-roots for quadratics:
    // alpha + beta = -b/a and alpha*beta = c/a  =>  c = a*(alpha*beta)
    double c_computed = static_cast<double>(a_loaded) * (alpha * beta);

    std::cout << "\nComputed values:\n";
    std::cout << "  alpha + beta = " << (alpha + beta)
              << " (should equal -b/a = " << (-(static_cast<double>(b_loaded))/a_loaded) << ")\n";
    std::cout << "  alpha * beta = " << (alpha * beta) << " (this equals c/a)\n";
    std::cout << "  Computed constant c = " << c_computed << "\n";

    // 5) Update JSON with computed c and write back
    loaded["polynomial"]["c"] = c_computed;
    {
        std::ofstream out(json_path);
        out << std::setw(2) << loaded << std::endl;
    }
    std::cout << "\nUpdated JSON with computed c written to " << json_path << "\n";

    return 0;
}
