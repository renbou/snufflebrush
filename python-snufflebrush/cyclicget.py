def cyclicgetlist(var, ind1, ind2):
    if ind2 < ind1:
        ind2 += len(var)
    l = [var[i%len(var)] for i in range(ind1, ind2)]
    return l
