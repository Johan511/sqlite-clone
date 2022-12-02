import enum
import random
import sys


class Types(enum.Enum):
    INTEGER = 1
    STRING = 2


class generateField:
    def __init__(self, options):
        self.options = options
        if "int_begin" not in options :
            self.options["int_begin"] = 1
        if "int_end" not in options :
            self.options["int_end"] = 10_000_000
        if "chars" not in options :
            self.options["chars"] = [chr(i) for i in range(
                ord('a'), ord('z'))] + [chr(i) for i in range(ord('A'), ord('Z'))]

    # def getRandom(self):
    #     if (self.type == Types.INTEGER):
    #         return self.genRandomInt()
    #     elif (self.type == Types.STRING):
    #         return self.genRandomStr()

    def getRandomInt(self):
        return random.randint(self.options["int_begin"], self.options["int_end"])

    def getRanomStr(self, str_length=10):
        r_string = ""
        for _ in range(0, str_length):
            r_string += self.options["chars"][random.randint(
                0, len(self.options["chars"]) - 1)]
        return r_string


class GenInput:
    def __init__(self, schema, num_inputs: int = 100, options={}, fileName: str = "input.txt"):
        self.randomGenerator = generateField(options)
        self.fileName = fileName
        self.file = open(fileName, 'w+')
        self.schema = schema
        self.num_inputs = num_inputs

    def oneLine(self):
        line = "insert "
        for i in self.schema:
            if (self.schema[i] == Types.INTEGER):
                line += str(self.randomGenerator.getRandomInt())
            elif (self.schema[i] == Types.STRING):
                line += str(self.randomGenerator.getRanomStr())
            line += " "
        line += '\n'
        return line

    def genInput(self):
        for i in range(self.num_inputs):
            self.file.write(self.oneLine())


if __name__ == '__main__':
    generator = GenInput(
        {"sno": Types.INTEGER,  "name": Types.STRING, "email":  Types.STRING}).genInput()
