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

// Evaluate condition: true only if "true" (case-insensitive)
bool evalCondition(const std::string& cond) {
    std::string lower = cond;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    return (lower == "true");
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

    // io.out([list]);
    if (trimmed.rfind("io.out(", 0) == 0 && trimmed.back() == ';') {
        size_t start_bracket = trimmed.find('[');
        size_t end_bracket = trimmed.rfind(']');
        if (start_bracket != std::string::npos && end_bracket != std::string::npos && end_bracket > start_bracket) {
            std::string list_content = trimmed.substr(start_bracket + 1, end_bracket - start_bracket - 1);
            auto items = parseList(list_content);

            if (!items.empty()) {
                for (size_t i = 0; i < items.size(); ++i) {
                    std::cout << items[i];
                    if (i + 1 < items.size()) std::cout << ", ";
                }
                std::cout << std::endl;
            } else {
                std::cout << "Syntax error in list" << std::endl;
            }
            return;
        }

        // io.out("text");
        size_t start_quote = trimmed.find('"');
        size_t end_quote = trimmed.rfind('"');
        if (start_quote != std::string::npos && end_quote != std::string::npos && end_quote > start_quote) {
            std::string message = trimmed.substr(start_quote + 1, end_quote - start_quote - 1);
            std::cout << message << std::endl;
            return;
        }

        // empty io.out();
        if (trimmed == "io.out();") {
            std::cout << std::endl;
            return;
        }

        std::cout << "Syntax error in io.out()" << std::endl;
    } else {
        std::cout << "Unknown command: " << trimmed << std::endl;
    }
}

int main() {
    std::cout << "Z shell\n";
    std::string z_logotop = " ^";
    std::string z_logo_m = "|Z|";
    std::string z_logobottom = " v";
    std::string line;

    bool inIfBlock = false;
    bool conditionTrue = false;

    while (true) {
        std::cout << ">> ";
        std::getline(std::cin, line);
        std::string input = trim(line);

        if (input == "q" || input == "quit") break;

        // Zlang logo commands always available
        if (input == "Zlang" || input == "#Zlangfun") {
            std::cout << z_logotop << "\n";
            std::cout << z_logo_m << "\n";
            std::cout << z_logobottom << "\n"; 
            continue;
        }
        if (input == "#tuxtux")
        {
            
            for (size_t i = 0; i < 10; i++)
            {
                //some fun with linux
            
                std::cout << "penguin\n";
                std::cout << "pengeon\n";
            }
        }
        
        if (!inIfBlock) {
            // detect if condition with { block
            if (input.rfind("if ", 0) == 0) {
                size_t brace_pos = input.find(" {");
                if (brace_pos == std::string::npos) {
                    std::cout << "Syntax error: missing ' {'\n";
                    continue;
                }
                std::string cond = trim(input.substr(3, brace_pos - 3));
                conditionTrue = evalCondition(cond);
                inIfBlock = true;
                continue;
            }

            // normal commands outside block
            if (input.rfind("io.out(", 0) == 0 && input.back() == ';') {
                runCommand(input);
            } else {
                std::cout << "Unknown command\n";
            }
        } else {
            // inside if block: end with '}'
            if (input == "}") {
                inIfBlock = false;
                conditionTrue = false;
                continue;
            }

            // if condition true, run commands in block
            if (conditionTrue) {
                runCommand(input);
            }
            // else skip commands inside false if-block
        }
    }
    return 0;
}
