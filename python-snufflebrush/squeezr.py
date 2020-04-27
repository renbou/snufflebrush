from zstd import ZstdCompressionDict as compDict, ZstdCompressor as comp, ZstdDecompressor as decomp
from os import remove as rmfile, mkdir, listdir as ls
from filetype import guess as typeof
class squeezr:
    def __init__(self, data):
        self.maxDictSize = 43*1000*1000
        self.data = data
        self.compressLevel = 22
        self.type = typeof(self.data)
        if self.type is None:
            self.type = 'no'
        else:
            self.type = self.type.extension
        try:
            with open('compDicts/'+self.type+'dict','rb') as inp:
                self.dictData = inp.read()
            if self.data not in self.dictData and len(self.dictData) < (self.maxDictSize if self.type != 'no' else self.maxDictSize * 2):
                self.dictData += self.data
        except:
            self.dictData = self.data
        self.compDict = compDict(self.data)

    def __del__(self):
        try:
            tmp = ls('compDicts')
        except:
            mkdir('compDicts')
        with open('compDicts/'+self.type+'dict','wb+') as out:
            out.write(self.dictData)

    def compressData(self):
        compressor = comp(level=self.compressLevel, dict_data=self.compDict)
        return compressor.compress(self.data)

    def decompressData(self):
        decompressor = decomp(dict_data = self.compDict)
        return decompressor.decompress(self.data)
