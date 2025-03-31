f = open("decoder.sp", "w")

f.write(".TITLE decoder\n")
f.write("\n\n")

f.write("*****************************\n")
f.write("**     Library setting     **\n")
f.write("*****************************\n")

f.write(".protect\n")
f.write(".include '7nm_TT.pm'\n")
f.write(".unprotect\n")
f.write("\n\n")


f.write("*****************************\n")
f.write("**      Input Vector       **\n")
f.write("*****************************\n")

f.write(".VEC 'decoder_pattern.vec'\n")
f.write("\n\n")


f.write("*****************************\n")
f.write("**     Voltage Source      **\n")
f.write("*****************************\n")

VDD = 0.7
f.write(".global VDD GND\n")
f.write("Vvdd VDD GND "+str(VDD)+"v\n")
f.write("\n\n")


f.write("*****************************\n")
f.write("**      Your Circuit       **\n")
f.write("*****************************\n")

f.write(".subckt inverter in out\n")
f.write("	Mp  out  in  VDD  x  pmos_lvt  m=1\n")
f.write("	Mn  out  in  GND  x  nmos_lvt  m=1\n")
f.write(".ends\n")
f.write("\n")

f.write(".subckt buffer in out\n")
f.write("	X_INV1 in   in_b inverter\n")
f.write("	X_INV2 in_b out  inverter\n")
f.write(".ends\n")
f.write("\n")

f.write(".subckt AND_3 INA INB INC OUT\n")
f.write("	mpA Y   INA VDD x pmos_lvt m=1\n")
f.write("	mpB Y   INB VDD x pmos_lvt m=1\n")
f.write("	mpC Y   INC VDD x pmos_lvt m=1\n")
f.write("	mnA nd1 INA GND x nmos_lvt m=1\n")
f.write("	mnB nd2 INB nd1 x nmos_lvt m=1\n")
f.write("	mnC Y   INC nd2 x nmos_lvt m=1\n")
f.write("	X_IN Y OUT inverter\n")
f.write(".ends\n")
f.write("\n")

f.write(".subckt DECODER24 INA INB EN OUT1 OUT2 OUT3 OUT4\n")
f.write("	X_INV1 INA INAI inverter\n")
f.write("	X_INV2 INB INBI inverter\n")
f.write("	X_INV3 INC INCI inverter\n")
f.write("	X_AND3_1 INAI INBI EN OUT1 AND_3\n")
f.write("	X_AND3_2 INAI INB  EN OUT2 AND_3\n")
f.write("	X_AND3_3 INA  INBI EN OUT3 AND_3\n")
f.write("	X_AND3_4 INA  INB  EN OUT4 AND_3\n")
f.write(".ends\n")
f.write("\n")


for i in range(6):
    f.write("X_BUF"+str(i)+" Address"+str(i)+" IN"+str(i)+"\n")
f.write("\n")
    
f.write("X_DEC11  IN4 IN5 Clock")
for i in range(4):
    f.write(" EN1"+str(i+1).ljust(2))
f.write(" DECODER24\n")

for i in range(4):
    f.write("X_DEC2"+str(i+1).ljust(2)+" IN2 IN3 EN1"+str(i+1).ljust(2))
    for j in range(4):
        f.write(" EN2"+str(i*4+j+1).ljust(2))
    f.write(" DECODER24\n")
    
for i in range(16):
    f.write("X_DEC3"+str(i+1).ljust(2)+" IN0 IN1 EN2"+str(i+1).ljust(2))
    for j in range(4):
        f.write(" Wordline"+str(64-(i*4+j+1)).ljust(2))
    f.write(" DECODER24\n")
f.write("\n")

Capacatance = (43.5*2/1000 + 0.4*0.162)*64
for i in range(64):
    f.write("C"+str(i).ljust(3)+"Wordline"+str(i).ljust(3)+"GND "+str(Capacatance)+"f\n")
f.write("\n")


f.write("\n\n")



f.write("*****************************\n")
f.write("**  Transient Analysis     **\n")
f.write("*****************************\n")
        
f.write(".tran 0.01ns 64ns\n")
f.write("\n\n")

f.write("*****************************\n")
f.write("**    Simulator setting    **\n")
f.write("*****************************\n")

f.write(".option post\n")
f.write(".options probe\n")
f.write(".probe v(*) i(*)\n")
f.write(".option captab\n")	
f.write(".TEMP 25\n")
f.write("\n\n")

f.write("*****************************\n")
f.write("**      Measurement        **\n")
f.write("*****************************\n")

f.write(".measure TRAN Static_pwr avg POWER from=0.0n to=64.0n\n")
f.write("\n\n")

f.write(".end\n")


f.close() 



f = open("decoder_pattern.vec", "w")

f.write("RADIX 1 1 1 1 1 1 1\n")
f.write("VNAME Clock Address5 Address4 Address3 Address2 Address1 Address0\n")
f.write("IO I I I I I I I\n")
f.write("tunit ns\n")
f.write("slope  0.05\n")
f.write("tdelay 0.0\n")
f.write("vih 0.7\n")
f.write("vil 0\n")
f.write("\n")


for i in range(64):
    binaryi = f'{i:06b}'
    
    f.write(str(i).rjust(2)+".0  0")
    for j in range(6):
        f.write(" "+binaryi[j])
    f.write("\n")
        
    f.write(str(i).rjust(2)+".5  1")
    for j in range(6):
        f.write(" "+binaryi[j])
    f.write("\n")

f.close()




   