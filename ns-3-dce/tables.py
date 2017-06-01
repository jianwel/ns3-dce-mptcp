def getNBits(mcs):
    if mcs<=28 and mcs >16:
        return 6
    elif mcs>9 and mcs<17:
        return 4
    elif mcs>0 and mcs<10:
        return 2


