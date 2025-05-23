#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <map>
#include <sstream>
#include <fstream>   // || For file loading
#include <filesystem> // || For directory handling (C++17+)

// || Trim whitespace from start and end of string
std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start])) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1])) --end;
    return s.substr(start, end - start);
}

// || Global variable store: variable name -> value
std::unordered_map<std::string, std::string> variables;

// || Function definitions store: function name -> function body
std::map<std::string, std::string> functions;

// || Replace all variable names with their current values inside an expression
std::string resolveVariables(const std::string& expr) {
    std::string resolved = expr;
    for (const auto& [key, val] : variables) {
        size_t pos;
        while ((pos = resolved.find(key)) != std::string::npos) {
            resolved.replace(pos, key.length(), val);
        }
    }
    return resolved;
}

// || Evaluate simple boolean conditions: supports ==, !=, >, < and literal true/false
bool evalCondition(const std::string& cond) {
    std::string c = resolveVariables(trim(cond));
    std::transform(c.begin(), c.end(), c.begin(), ::tolower);

    if (c == "true") return true;
    if (c == "false") return false;

    size_t pos;

    if ((pos = c.find("==")) != std::string::npos) {
        int lhs = std::stoi(trim(c.substr(0, pos)));
        int rhs = std::stoi(trim(c.substr(pos + 2)));
        return lhs == rhs;
    }
    if ((pos = c.find("!=")) != std::string::npos) {
        int lhs = std::stoi(trim(c.substr(0, pos)));
        int rhs = std::stoi(trim(c.substr(pos + 2)));
        return lhs != rhs;
    }
    if ((pos = c.find(">")) != std::string::npos) {
        int lhs = std::stoi(trim(c.substr(0, pos)));
        int rhs = std::stoi(trim(c.substr(pos + 1)));
        return lhs > rhs;
    }
    if ((pos = c.find("<")) != std::string::npos) {
        int lhs = std::stoi(trim(c.substr(0, pos)));
        int rhs = std::stoi(trim(c.substr(pos + 1)));
        return lhs < rhs;
    }

    return false;
}

// || Parse comma-separated list items, supports quoted strings and trimming
std::vector<std::string> parseList(const std::string& s) {
    std::vector<std::string> items;
    size_t pos = 0;

    while (pos < s.size()) {
        while (pos < s.size() && std::isspace(s[pos])) pos++;
        if (pos >= s.size()) break;

        if (s[pos] == '"') {
            size_t start = ++pos;
            size_t end = s.find('"', start);
            if (end == std::string::npos) break;
            items.push_back(s.substr(start, end - start));
            pos = end + 1;
        } else {
            size_t start = pos;
            while (pos < s.size() && s[pos] != ',' && s[pos] != ']') pos++;
            items.push_back(trim(s.substr(start, pos - start)));
        }

        while (pos < s.size() && std::isspace(s[pos])) pos++;
        if (pos < s.size() && s[pos] == ',') pos++;
    }

    return items;
}

// || Run a single Zlang command line
void runCommand(const std::string& line) {
    std::string trimmed = trim(line);

    // || Function call: name();
    if (trimmed.find("();") != std::string::npos) {
        std::string fname = trimmed.substr(0, trimmed.find("();"));
        if (functions.find(fname) != functions.end()) {
            runCommand(functions[fname]);
        } else {
            std::cout << "Unknown function: " << fname << std::endl;
        }
        return;
    }

    // || Variable declaration: var x = value;
    if (trimmed.rfind("var ", 0) == 0) {
        size_t eq = trimmed.find('=');
        if (eq != std::string::npos) {
            std::string name = trim(trimmed.substr(4, eq - 4));
            std::string value = trim(trimmed.substr(eq + 1));
            variables[name] = value;
        } else {
            std::cout << "Syntax error in variable declaration\n";
        }
        return;
    }

    // || io.out(...) command for output
    if (trimmed.rfind("io.out(", 0) == 0 && trimmed.back() == ';') {
        size_t start = trimmed.find('(');
        size_t end = trimmed.rfind(')');
        if (start == std::string::npos || end == std::string::npos || end <= start) {
            std::cout << "Syntax error in io.out()\n";
            return;
        }

        std::string content = trim(trimmed.substr(start + 1, end - start - 1));
        content = resolveVariables(content);

        if (content.empty()) {
            std::cout << std::endl;
            return;
        }

        // || Output list elements if content is [ ... ]
        if (content.front() == '[' && content.back() == ']') {
            auto items = parseList(content.substr(1, content.size() - 2));
            for (size_t i = 0; i < items.size(); ++i) {
                std::cout << items[i];
                if (i + 1 < items.size()) std::cout << ", ";
            }
            std::cout << std::endl;
            return;
        }

        // || Output quoted strings without quotes
        if (content.front() == '"' && content.back() == '"') {
            std::cout << content.substr(1, content.size() - 2) << std::endl;
            return;
        }

        // || Output numbers
        try {
            size_t pos;
            double num = std::stod(content, &pos);
            if (pos == content.size()) {
                std::cout << num << std::endl;
                return;
            }
        } catch (...) {}

        std::cout << "Syntax error in io.out()\n";
        return;
    }

    std::cout << "Unknown command: " << trimmed << std::endl;
}

int main() {
    std::cout << "Zlang shell (type q or quit to exit)\n";

    std::string line;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, line);
        std::string input = trim(line);

        if (input == "q" || input == "quit") break;

        // || Load a .zl script file: load filename.zl
        if (input.rfind("load ", 0) == 0) {
            std::string filename = trim(input.substr(5));

            // || Append .zl extension if missing
            if (filename.size() < 3 || filename.substr(filename.size() - 3) != ".zl") {
                filename += ".zl";
            }

            // || Check if file exists before opening
            if (!std::filesystem::exists(filename)) {
                std::cout << "File not found: " << filename << std::endl;
                continue;
            }

            std::ifstream file(filename);
            if (!file) {
                std::cout << "Could not open file: " << filename << std::endl;
                continue;
            }
            std::string fileLine;
            while (std::getline(file, fileLine)) {
                fileLine = trim(fileLine);
                if (!fileLine.empty()) runCommand(fileLine);
            }
            file.close();
            continue;
        }

        // || Function definition: func name() { ... }
        if (input.rfind("func ", 0) == 0) {
            size_t nameStart = 5;
            size_t paren = input.find("()", nameStart);
            size_t blockStart = input.find('{');
            size_t blockEnd = input.find('}');

            if (paren != std::string::npos && blockStart != std::string::npos && blockEnd != std::string::npos) {
                std::string fname = trim(input.substr(nameStart, paren - nameStart));
                std::string body = trim(input.substr(blockStart + 1, blockEnd - blockStart - 1));
                functions[fname] = body;
            } else {
                std::cout << "Syntax error in func definition\n";
            }
            continue;
        }

        // || Inline if statement: if cond { ... }
        if (input.rfind("if ", 0) == 0) {
            size_t open = input.find('{');
            size_t close = input.find('}');
            if (open != std::string::npos && close != std::string::npos && close > open) {
                std::string cond = trim(input.substr(3, open - 3));
                std::string block = trim(input.substr(open + 1, close - open - 1));
                if (evalCondition(cond)) runCommand(block);
                continue;
            } else {
                std::cout << "Syntax error: if statement must use { ... }\n";
                continue;
            }
        }

        // || Loop statement: loop N { ... }
        if (input.rfind("loop ", 0) == 0) {
            size_t space = input.find(' ', 5);
            size_t open = input.find('{');
            size_t close = input.find('}');
            if (space != std::string::npos && open != std::string::npos && close != std::string::npos && close > open) {
                std::string countStr = trim(input.substr(5, open - 5));
                std::string block = trim(input.substr(open + 1, close - open - 1));
                int count = 0;
                try {
                    count = std::stoi(resolveVariables(countStr));
                } catch (...) {
                    std::cout << "Invalid loop count\n";
                    continue;
                }
                for (int i = 0; i < count; ++i) {
                    runCommand(block);
                }
            } else {
                std::cout << "Syntax error: loop must be like 'loop N { ... }'\n";
            }
            continue;
        }

        // || Otherwise, run as a command
        runCommand(input);
    }

    return 0;
}
