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

#!/usr/bin/env python
# -*- coding: utf-8 -*-
def checkParentheses(s):
    stack = []
    openningParentheses = ['{', '[', '(']
    closingParentheses = ['}', ']', ')']
    pairs = {'}':'{', ']':'[', ')':'('}
    for ch in s:
        if ch in openningParentheses:
            stack.append(ch)
        elif (ch in closingParentheses):
            if (len(stack) == 0):
                return False
            translatedCh = pairs[ch]
            stackTop = stack[-1]
            if (stackTop == translatedCh):
                stack.pop()
            else:
                return False

    return (len(stack) == 0)


testLines =  ['1{2}3', '1(2)3', '1[2]3', '1[2(3)4]5', '1{2(3}4)5', "1}2{3", "1{"]
for line in testLines:
    print line, ":", checkParentheses(line)

