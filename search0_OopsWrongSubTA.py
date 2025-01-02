# https://www.reddit.com/r/askmath/comments/1hqjtej/what_answer_is_closest_to_zero/m4tmq6y/
from itertools import permutations

for a,b,c,d,e,f,g,h,i,j in permutations(range(10)):
    if 0 in (a, d, f, i): continue
    if (100*a+10*b+c)*(10*d+e)-(100*f+10*g+h)*(10*i+j) == 0: print(a,b,c,d,e,f,g,h,i,j)
