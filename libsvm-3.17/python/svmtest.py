from svmutil import *
import sys

def avg(x):
    n=0
    for i in range(len(x)):
        n+=x[i]
    n=n/len(x)
    return n

#4 datensaetze, Cross validation: Erstes 10tel einlesen, letztes 10tel klassifizieren
#Zweites 10tel einlesen, letztes klassifizieren...
ty,tx = svm_read_problem(sys.argv[1])
l=len(ty)
blen=l//10
gen=[]
for i in range(10):
    m = svm_train(ty[i*blen:blen*(i+1)],tx[i*blen:blen*(i+1)],'-c 4') #modell aus testmenge bilden
    gen.append(svm_predict(ty[-10:],tx[-10:],m)[1][0])
print("\n\n\n\nDurchschnittliche Genauigkeit: "+str(avg(gen)))
