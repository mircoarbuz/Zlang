#!/usr/bin/env python3

import os, sys, readline

variables = {}
functions = {}

def trim(s):
    return s.strip()

def resolve_vars(expr):
    for k, v in variables.items():
        expr = expr.replace(k, v)
    return expr

def eval_condition(c):
    c = resolve_vars(c.strip().lower())
    if c == "true": return True
    if c == "false": return False
    for op in ["==", "!=", ">", "<"]:
        if op in c:
            lhs, rhs = map(str.strip, c.split(op))
            try:
                lhs, rhs = int(lhs), int(rhs)
                return eval(f"{lhs}{op}{rhs}")
            except:
                return False
    return False

def run_command(line):
    line = trim(line)
    if not line: return

    if line.endswith("();"):
        fname = line[:-3]
        if fname in functions:
            run_command(functions[fname])
        else:
            print(f"Unknown function: {fname}")
        return

    if line.startswith("var "):
        parts = line[4:].split("=", 1)
        if len(parts) == 2:
            name, val = map(trim, parts)
            variables[name] = val
        else:
            print("Syntax error in var declaration")
        return

    if line.startswith("io.out(") and line.endswith(");"):
        content = line[7:-2].strip()
        content = resolve_vars(content)
        if content.startswith('"') and content.endswith('"'):
            print(content[1:-1])
        else:
            try:
                print(eval(content))
            except:
                print("Output:", content)
        return

    if line.startswith("func "):
        name = line[5:line.find("()")].strip()
        block = line[line.find("{")+1:line.find("}")].strip()
        functions[name] = block
        return

    if line.startswith("if "):
        cond = line[3:line.find("{")].strip()
        block = line[line.find("{")+1:line.find("}")].strip()
        if eval_condition(cond):
            run_command(block)
        return

    if line.startswith("loop "):
        parts = line.split("{")
        count = int(resolve_vars(parts[0][5:].strip()))
        block = parts[1].split("}")[0].strip()
        for _ in range(count):
            run_command(block)
        return

    print("Unknown command:", line)

def main():
    print("Zlang (Python Version) - type 'quit' to exit")
    while True:
        try:
            line = input(">> ")
            if line.strip() in ("q", "quit"):
                break
            run_command(line)
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()

