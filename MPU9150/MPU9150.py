class MPU9150:
   def __init__(self, port='COM12'):
      # connect to serial port
      import serial
      self.ser = serial.Serial(port,19200,timeout=1)
      # IDK what is written but get rid of it
      self.ser.read()
      self.ser.read()
   
   def close(self):
      self.ser.close()

   def yieldData(self,what='X'):
      while True:
         yield self.get(what)

   def get(self,what='X'):
      import struct
      if what.islower():
         if what.lower() == 'x':
           self.ser.write(b'x')
         elif what.lower() == 'y':
           self.ser.write(b'y')
         elif what.lower() == 'z':
           self.ser.write(b'z')
         else:
           self.ser.write(b'x')
      else:
         if what.lower() == 'x':
           self.ser.write(b'X')
         elif what.lower() == 'y':
           self.ser.write(b'Y')
         elif what.lower() == 'z':
           self.ser.write(b'Z')
         else:
           self.ser.write(b'X')

      xh = self.ser.read()
      xl = self.ser.read()
      x_ =  b""
      x_ += xh
      x_ += xl
      x = struct.unpack('>h',x_)
      return x[0]
      
   def stream(self,what='x',values=10):
      for x in range(0,values):
              x = self.get(what)
              print(x)

   # to accomodate the plotting function we need these emit functions with no arguments 
   def emitX(self):
      while True:
         a = self.get('X')
         yield a
   
   def emitx(self):
      while True:
         a = self.get('x')
         yield a
   
   def emitY(self):
      while True:
         a = self.get('Y')
         yield a
   
   def emity(self):
      while True:
         a = self.get('y')
         yield a
   
   def emitZ(self):
      while True:
         a = self.get('Z')
         yield a
   
   def emitz(self):
      while True:
         a = self.get('z')
         yield a
   
   def emitT(self):
      while True:
         a = self.get('T')
         yield a
   
