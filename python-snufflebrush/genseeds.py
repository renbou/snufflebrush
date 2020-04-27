from numpy.random import seed as npseed
from numpy.random import RandomState as rndgen
from numpy.random import shuffle as npshuffle
from numpy import int64
from bitarray import bitarray
from cyclicget import cyclicgetlist as getl
from removepart import removepart as remp
from nextprime import next_prime
from timeit import default_timer as timer

def cyclicxor(var1, var2, ind1):
    i1 = ind1
    for i2 in range(len(var2)):
        var1[i1%len(var1)] ^= var2[i2]
        i1 += 1
    return var1

class seedgen:
    def newseed(self, baseloc, xorsz, xorspace):
        tempmain = self.mainseed
        tempseed = bitarray(getl(tempmain, baseloc, baseloc+self.seedsz))
        tempmain = remp(tempmain, baseloc, baseloc+self.seedsz)
        xorshift = 0
        xorval = 0
        while len(tempmain) > xorsz:
            xorval = tempmain[:xorsz]
            """
            if xorshift+xorsz > 64:
                cycloc = xorsz-((xorshift+xorsz)%64)
                xorval = tempmain[cycloc:xorsz]+bitarray('0'*(64-xorsz))+tempmain[:cycloc]
            else:
                xorval = bitarray('0'*xorshift)+tempmain[:xorsz]+bitarray('0'*64-xorsz-xorshift)
            tempseed ^= xorval
            """
            tempseed = cyclicxor(tempseed, xorval, xorshift)
            xorshift += xorspace + xorsz
            tempmain = tempmain[xorsz:]
        if len(tempmain) > 0:
            tempseed = cyclicxor(tempseed, tempmain, xorshift)
        self.seed = tempseed
        
    def __init__(self, password):
        password += "yozh"
        if len(password.encode('utf-8'))-4 < 8:
            for i in range(1, 8-len(password)+4+1):
                password += str(i)
        passbits = bitarray(endian='big')
        passbits.frombytes(password.encode('utf-8'))
        shuffled_bits = [int(i) for i in str(passbits)[10:-2]]
        npseed(int.from_bytes(passbits[::2][:32].tobytes(), byteorder='big'))
        npshuffle(shuffled_bits)
        self.mainseed = bitarray(shuffled_bits)
        self.seedsz = 32
        self.newseed(0, 4, 4)
        self.minval = -2**63
        self.maxval = 2**63
    def gennext(self):
        rnd = rndgen(int.from_bytes(self.seed.tobytes(), byteorder='big'))
        num = int(rnd.randint(self.minval, self.maxval, dtype=int64))
        xorshift = next_prime(abs(num))
        if xorshift > self.maxval:
            xorshift %= self.maxval
            xorshift = next_prime(xorshift)
        xorshift %= len(self.mainseed)
        xorwith = bitarray(endian='big')
        xorwith.frombytes((num&0xffffffffffffffff).to_bytes(8, byteorder='big'))
        self.mainseed = cyclicxor(self.mainseed, xorwith, xorshift)
        xorsz = rnd.randint(1,9)
        xorspace = rnd.randint(1,9)
        baseloc = rnd.randint(len(self.mainseed))
        self.newseed(baseloc, xorsz, xorspace)
        return num

def main():
    start = timer()
    psw = "lolkek"
    sg = seedgen(psw)
    for i in range(900):
        x = sg.gennext()
        #print(sg.gennext(),end=" ")
    end = timer()
    print('Exec time:',end-start)


if __name__ == '__main__':
  main()