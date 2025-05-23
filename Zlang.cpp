#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <unordered_map>
#include <map>
#include <sstream>

std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start])) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1])) --end;
    return s.substr(start, end - start);
}

std::unordered_map<std::string, std::string> variables;
std::map<std::string, std::string> functions;

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

void runCommand(const std::string& line) {
    std::string trimmed = trim(line);

    // >> Function call
    if (trimmed.find("();") != std::string::npos) {
        std::string fname = trimmed.substr(0, trimmed.find("();"));
        if (functions.find(fname) != functions.end()) {
            runCommand(functions[fname]);
        } else {
            std::cout << "Unknown function: " << fname << std::endl;
        }
        return;
    }

    // >> Variable declaration
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

    // >> io.out(...)
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

        if (content.front() == '[' && content.back() == ']') {
            auto items = parseList(content.substr(1, content.size() - 2));
            for (size_t i = 0; i < items.size(); ++i) {
                std::cout << items[i];
                if (i + 1 < items.size()) std::cout << ", ";
            }
            std::cout << std::endl;
            return;
        }

        if (content.front() == '"' && content.back() == '"') {
            std::cout << content.substr(1, content.size() - 2) << std::endl;
            return;
        }

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
    std::cout << "Zlang shell\n";

    std::string line;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, line);
        std::string input = trim(line);

        if (input == "q" || input == "quit") break;

        // >> func definition
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

        // >> if statement
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

        // >> loop statement
        if (input.rfind("loop ", 0) == 0) {
            size_t open = input.find('{');
            size_t close = input.find('}');
            if (open != std::string::npos && close != std::string::npos && close > open) {
                std::string countStr = trim(input.substr(5, open - 5));
                std::string block = trim(input.substr(open + 1, close - open - 1));

                int count = 0;
                try {
                    count = std::stoi(resolveVariables(countStr));
                } catch (...) {
                    std::cout << "Syntax error: invalid loop count\n";
                    continue;
                }

                for (int i = 0; i < count; ++i) {
                    runCommand(block);
                }
                continue;
            } else {
                std::cout << "Syntax error: loop statement must use { ... }\n";
                continue;
            }
        }

        runCommand(input);
    }

    return 0;
}
