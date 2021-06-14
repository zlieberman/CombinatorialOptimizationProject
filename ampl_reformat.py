# converts the .txt instances into .dat files
# compatible with ampl
def ampl_reformat():
    for ii in range(1,9):
        for jj in range(1,11):
            f = open('data/extracted/'+str(ii)+'/'+str(jj)+'.txt','r')
            f1 = open('data/extracted/'+str(ii)+'/'+str(jj)+'.dat','w')
            lines = f.readlines()
            params = lines[1].split()
            f1.write('param N := '+params[0]+';\n')
            f1.write('param M := '+params[0]+';\n')
            f1.write('param C := '+params[1]+';\n')
            f1.write('param sizes := '+'\n')
            for ll in range(2,len(lines)):
                f1.write(str(ll-1)+' '+lines[ll])
            f1.write(";\n")
            f1.close()
            f.close()

if __name__ == '__main__':
    ampl_reformat()

