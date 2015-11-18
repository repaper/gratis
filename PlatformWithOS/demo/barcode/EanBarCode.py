"""
Class for generating EAN bar codes
requires Python Imaging Library (PIL)

If the number does not have a checksum digit (i.e. a 12 digit value)
it is added automatically to make the code 13 digits long.

Originally downloaded from:

  http://code.activestate.com/recipes/426069-ean-bar-code-image-generator/

"""

class EanBarCode:
    """ Compute the EAN bar code """
    def __init__(self):
        A = {0 : '0001101', 1 : '0011001', 2 : '0010011', 3 : '0111101', 4 : '0100011',
             5 : '0110001', 6 : '0101111', 7 : '0111011', 8 : '0110111', 9 : '0001011'}
        B = {0 : '0100111', 1 : '0110011', 2 : '0011011', 3 : '0100001', 4 : '0011101',
             5 : '0111001', 6 : '0000101', 7 : '0010001', 8 : '0001001', 9 : '0010111'}
        C = {0 : '1110010', 1 : '1100110', 2 : '1101100', 3 : '1000010', 4 : '1011100',
             5 : '1001110', 6 : '1010000', 7 : '1000100', 8 : '1001000', 9 : '1110100'}
        self.groupC = C

        self.family = {0 : (A,A,A,A,A,A), 1 : (A,A,B,A,B,B), 2 : (A,A,B,B,A,B), 3 : (A,A,B,B,B,A), 4 : (A,B,A,A,B,B),
                       5 : (A,B,B,A,A,B), 6 : (A,B,B,B,A,A), 7 : (A,B,A,B,A,B), 8 : (A,B,A,B,B,A), 9 : (A,B,B,A,B,A)}


    def makeCode(self, code):
        """ Create the binary code
        return a string which contains '0' for white bar, '1' for black bar, 'L' for long bar """

        # Convert code string in integer list
        self.EAN13 = []
        for digit in code:
            self.EAN13.append(int(digit))

        # If the code has already a checksum
        if len(self.EAN13) == 13:
            # Verify checksum
            self.verifyChecksum(self.EAN13)
            # If the code has not yet checksum
        elif len(self.EAN13) == 12:
            # Add checksum value
            self.EAN13.append(self.computeChecksum(self.EAN13))

        # Get the left codage class
        left = self.family[self.EAN13[0]]

        # Add start separator
        strCode = 'L0L'

        # Compute the left part of bar code
        for i in range(0, 6):
            strCode += left[i][self.EAN13[i+1]]

        # Add middle separator
        strCode += '0L0L0'

        # Compute the right codage class
        for i in range (7, 13):
            strCode += self.groupC[self.EAN13[i]]

        # Add stop separator
        strCode += 'L0L'

        return strCode


    def computeChecksum(self, arg):
        """ Compute the checksum of bar code """
        # UPCA/EAN13
        weight = [1, 3] * 6
        magic = 10
        sum = 0

        # checksum based on first 12 digits.
        for i in range(12):
            sum = sum + int(arg[i]) * weight[i]

        # compute sum
        z = ( magic - (sum % magic) ) % magic
        if z < 0 or z >= magic:
            return None
        return z


    def verifyChecksum(self, bits):
        """ Verify the checksum """
        computedChecksum = self.computeChecksum(bits[:12])
        codeBarChecksum = bits[12]

        if codeBarChecksum != computedChecksum:
            raise Exception('Bad checksum is {f:s} and should be {c:s}'.format(f=codeBarChecksum, c=computedChecksum))


    def drawBarCode(self, value, draw, x, y, height = 30):
        """ Convert string value to bar code starting at (x,y)
        and draw onto a PIL image
        height specifies in pixel of the bar code """

        import ImageFont

        # Get the bar code list
        bits = self.makeCode(value)

        # Get the bar code with the checksum added
        code = ''
        for digit in self.EAN13:
            code += '{n:d}'.format(n=digit)

        # Load font
        font = ImageFont.load('courB08.pil')

        # Draw first part of number
        draw.text((x, y + height - 9), code[0], font=font, fill=0)

        # Draw first part of number
        draw.text((x + 15, y + height - 9), code[1:7], font=font, fill=0)

        # Draw second part of number
        draw.text((len(bits) / 2 + 14 + x, y + height - 9), code[7:], font=font, fill=0)

        # Draw the bar codes
        for bit in range(len(bits)):
            # Draw normal bar
            if bits[bit] == '1':
                draw.rectangle(((bit + x + 8, y), (bit + x + 8, y + height - 10)), fill=0)
            # Draw long bar
            elif bits[bit] == 'L':
                draw.rectangle(((bit + x + 8, y), (bit + x + 8, y + height - 3)), fill=0)
