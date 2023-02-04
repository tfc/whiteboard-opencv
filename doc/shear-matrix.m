pkg load symbolic

syms W H N

x1 = [W; H]
x2 = [W; - H]

M = transpose(transpose([ x1, x2 ]) \ transpose([ x1, N *x2 ]))
