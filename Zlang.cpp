#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

// Trim whitespace from start and end
std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(s[start])) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(s[end - 1])) --end;
    return s.substr(start, end - start);
}

// Evaluate condition (support true, false, and simple integer comparisons)
bool evalCondition(const std::string& cond) {
    std::string c = trim(cond);
    std::transform(c.begin(), c.end(), c.begin(), ::tolower);

    if (c == "true") return true;
    if (c == "false") return false;

    // Support simple integer comparisons: >, <, ==, !=
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

    return false; // fallback
}

// Parse comma-separated list items inside brackets (supports quoted strings or numbers)
std::vector<std::string> parseList(const std::string& s) {
    std::vector<std::string> items;
    size_t pos = 0;

    while (pos < s.size()) {
        while (pos < s.size() && std::isspace(s[pos])) pos++;

        if (pos >= s.size()) break;

        if (s[pos] == '"') {
            size_t start_quote = pos + 1;
            size_t end_quote = s.find('"', start_quote);
            if (end_quote == std::string::npos) break;
            items.push_back(s.substr(start_quote, end_quote - start_quote));
            pos = end_quote + 1;
        } else {
            size_t start_token = pos;
            while (pos < s.size() && s[pos] != ',' && s[pos] != ']') pos++;
            std::string token = trim(s.substr(start_token, pos - start_token));
            if (!token.empty()) items.push_back(token);
        }

        while (pos < s.size() && std::isspace(s[pos])) pos++;
        if (pos < s.size() && s[pos] == ',') pos++;
    }

    return items;
}

void runCommand(const std::string& line) {
    std::string trimmed = trim(line);

    if (trimmed.rfind("io.out(", 0) == 0 && trimmed.back() == ';') {
        size_t start_paren = trimmed.find('(');
        size_t end_paren = trimmed.rfind(')');
        if (start_paren == std::string::npos || end_paren == std::string::npos || end_paren <= start_paren) {
            std::cout << "Syntax error in io.out()\n";
            return;
        }

        std::string content = trim(trimmed.substr(start_paren + 1, end_paren - start_paren - 1));

        if (content.empty()) {
            std::cout << std::endl;
            return;
        }

        // List case: [ ... ]
        if (content.front() == '[' && content.back() == ']') {
            auto items = parseList(content.substr(1, content.size() - 2));
            for (size_t i = 0; i < items.size(); ++i) {
                std::cout << items[i];
                if (i + 1 < items.size()) std::cout << ", ";
            }
            std::cout << std::endl;
            return;
        }

        // Quoted string case
        if (content.front() == '"' && content.back() == '"') {
            std::string message = content.substr(1, content.size() - 2);
            std::cout << message << std::endl;
            return;
        }

        // Number case (int or float)
        try {
            size_t pos = 0;
            double num = std::stod(content, &pos);
            if (pos == content.size()) {
                std::cout << num << std::endl;
                return;
            }
        } catch (...) {
            // Not a number, fall through
        }

        std::cout << "Syntax error in io.out()" << std::endl;
    } else {
        std::cout << "Unknown command: " << trimmed << std::endl;
    }
}

int main() {
    std::cout << "Zlang shell\n";

    std::string line;
    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, line);
        std::string input = trim(line);

        if (input == "q" || input == "quit") break;

        // Inline if support (only one-line if {...})
        if (input.rfind("if ", 0) == 0) {
            size_t brace_open = input.find('{');
            size_t brace_close = input.find('}');

            if (brace_open != std::string::npos && brace_close != std::string::npos && brace_close > brace_open) {
                std::string cond = trim(input.substr(3, brace_open - 3));
                std::string block = trim(input.substr(brace_open + 1, brace_close - brace_open - 1));

                if (evalCondition(cond)) {
                    runCommand(block);
                }
                continue;
            } else {
                std::cout << "Syntax error: if statement must have { ... } block on same line\n";
                continue;
            }
        }

        runCommand(input);
    }

    return 0;
}
