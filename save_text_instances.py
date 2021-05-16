
for i in range(1,9):
    f = open('/Users/paullevy/Desktop/Combinatorial Optimization/Project/Project 01/data/binpack' + str(i) +'.txt','r')
    count = 0
    string = ''
    for line in f:
        if 'u' in line or 't' in line:
            if count != 0:
                f2 = open('/Users/paullevy/Desktop/Combinatorial Optimization/Project/Project 01/data/extracted/'+str(i)+'/' + str(count) + '.txt', 'w')
                f2.write(string)
                f2.close()
            string = ''
            count += 1
        string += line
    f2 = open('/Users/paullevy/Desktop/Combinatorial Optimization/Project/Project 01/data/extracted/'+str(i)+'/' + str(count) + '.txt', 'w')
    f2.write(string)
    f2.close()
    f.close()


# print(string)