def removepart(var, ind1, ind2):
    ind1 = ind1%len(var)
    ind2 = ind2%len(var)
    if (ind2 < ind1):
        var = var[ind2:ind1]
    else:
        var = var[:ind1]+var[ind2:]
    return var
