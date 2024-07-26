#!/usr/bin/env python
# -*- coding: utf-8 -*-

'''
Implement function check (text) which checks whether brackets within text are
correctly nested. You need to consider brackets of three kinds: (), [], {}.


Examples:

check("a(b)") -> true
check("[{}]") -> true
check("[(]") -> false
check("}{") -> false
check("z([{}-()]{a})") -> true
check("") -> true
'''

def rmMatch(stack, ch):
    if (ch == '}'):
        if (len(stack) == 0):
            return False
        stackTop = stack[-1]
        if (stackTop != '{'):
            return False
        else:
            stack.pop()
    elif (ch == ')'):
        if (len(stack) == 0):
            return False
        stackTop = stack[-1]
        if (stackTop != '('):
            return False
        else:
            stack.pop()
    elif (ch == ']'):
        if (len(stack) == 0):
            return False
        stackTop = stack[-1]
        if (stackTop != '['):
            return False
        else:
            stack.pop()
    else:
        return None
    return True
    
    
def checkParentheses(s):
    stack = []
    for ch in s:
        if ((ch == '{') or (ch == '[') or (ch == '(')):
            stack.append(ch)
        else:
            res = rmMatch(stack, ch)
            if (res == True):
                pass
            elif (res == False):
                return False

    return (len(stack) == 0)


testLines =  ['1{2}3', '1(2)3', '1[2]3', '1[2(3)4]5', '1{2(3}4)5', "1}2{3", "1{", "", "[(]", "z([{}-()]{a})", "[(]"]
for line in testLines:
    print "'", line, "'", ":", checkParentheses(line)

