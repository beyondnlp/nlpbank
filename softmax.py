import math 

def EXP( b ):
    alpha=2.7182818284590452353602874713526624977 # =e^1
    return alpha ** b

def get_max( a, b ):
    if a > b : return a
    return b

def softmax_max(x):

    max_one = -math.inf
    for nx in x:
        if nx > max_one : max_one = nx

    exp_x=[]
    for nx in x:
        new_nx = nx-max_one
        e1=EXP( new_nx )
        exp_x.append( e1 )
    sum_exp_x = 1e-10
    for i in exp_x: sum_exp_x = sum_exp_x + i
    y=[]
    for es in exp_x: y.append(es/sum_exp_x)
    return y


def softmax_online(x):
    N = len(x)

    # Initialize variables
    m = [-1] * N
    d = [0] * N
    y = [0] * N

    old_max = -math.inf
    new_max = -math.inf
    exp_sum_x = 0;
    for i in range(N):
        new_max = get_max( old_max, x[i] )
        a=EXP( old_max - new_max )
        b=EXP( x[i] - new_max )
        exp_sum_x = exp_sum_x * a + b
        old_max = new_max

    for i in range(N): y[i] = EXP( x[i] - old_max ) / exp_sum_x;

    return y


m=[3, 5, 1]

print(softmax_online(m))


ret2=softmax_max(m)
print(ret2)