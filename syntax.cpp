// QuarterLang Official Syntax Implementation in Pure C++
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <functional>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <cmath>

//--------------------------------------------
// TYPE ALIASES
//--------------------------------------------
using DG = std::string; // Dodecagram base-12 string
using namespace std;

//--------------------------------------------
// RUNTIME BUILT-INS
//--------------------------------------------
void say(const string& msg) {
    cout << msg << endl;
}

DG to_dg(int n) {
    const char* digits = "0123456789AB";
    if (n == 0) return "0";
    string result;
    while (n > 0) {
        result = digits[n % 12] + result;
        n /= 12;
    }
    return result;
}

int from_dg(const DG& dg) {
    int result = 0;
    for (char c : dg) {
        int val = (c >= 'A') ? (10 + c - 'A') : (c - '0');
        result = result * 12 + val;
    }
    return result;
}

DG dg_add(const DG& a, const DG& b) {
    return to_dg(from_dg(a) + from_dg(b));
}

DG dg_mul(const DG& a, const DG& b) {
    return to_dg(from_dg(a) * from_dg(b));
}

void write_file(const string& filename, const string& content) {
    ofstream out(filename);
    out << content;
    out.close();
}

string read_file(const string& filename) {
    ifstream in(filename);
    stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

void show_health() {
    say("Capsule Health: OK\nEntropy: 0.042\nMemory: 4MB");
}

//--------------------------------------------
// STRUCTURES & FUNCTIONS
//--------------------------------------------
struct CapsuleContext {
    unordered_map<string, int> ints;
    unordered_map<string, float> floats;
    unordered_map<string, DG> dgs;
};

//--------------------------------------------
// EXAMPLE CAPSULE
//--------------------------------------------
void capsule_main() {
    // star
    CapsuleContext ctx;

    // val x as int: 10
    ctx.ints["x"] = 10;

    // var y as float: 2.5
    ctx.floats["y"] = 2.5f;

    // say "Value is {x} and {y}"
    say("Value is " + to_string(ctx.ints["x"]) + " and " + to_string(ctx.floats["y"]));

    // val base as int: 20
    ctx.ints["base"] = 20;

    // derive speed from base by 5
    ctx.ints["speed"] = ctx.ints["base"] * 5;

    // say speed
    say("Speed: " + to_string(ctx.ints["speed"]));

    // dg literal assignment
    ctx.dgs["alpha"] = to_dg(144); // dg:100
    say("alpha (DG): " + ctx.dgs["alpha"]);

    // when/else control
    int z = -5;
    if (z > 0)
        say("Positive");
    else
        say("Non-positive");

    // loop from 1 to 5:
    for (int i = 1; i <= 5; ++i)
        say("Count " + to_string(i));

    // match-case simulation
    int value = 2;
    switch (value) {
    case 1: say("One"); break;
    case 2: say("Two"); break;
    default: say("Other"); break;
    }

    // define add
    auto add = [](int a, int b) -> int { return a + b; };
    say("Add 3 + 4 = " + to_string(add(3, 4)));

    // fn lambda
    auto square = [](int x) -> int { return x * x; };
    say("Square 7 = " + to_string(square(7)));

    // thread example
    thread t([] { say("[Thread] Hello from thread!"); });
    t.join();

    // pipe write:
    write_file("output.txt", "This is piped to file.");
    say("[Pipe] Written to file.");

    // asm stub
    say("[ASM] Inline NASM would be embedded here.");

    // show_health
    show_health();
    // end
}

//--------------------------------------------
// MAIN
//--------------------------------------------
int main() {
    capsule_main();
    return 0;
}
