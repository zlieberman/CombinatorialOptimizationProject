# creates .run files to run ampl on a problem instance
def generate_ampl_runfile():
    for ii in range(1,9):
        for jj in range(1,11):
            f = open('data/extracted/'+str(ii)+'/'+str(jj)+'.run','w')
            f.write('model binpack.mod;\n')
            f.write('data data/extracted/'+str(ii)+'/'+str(jj)+'.dat;\n')
            f.write('solve;\n')
            f.write('reset;\n')
            f.close()

if __name__ == '__main__':
    generate_ampl_runfile()