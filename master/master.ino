String retSymb = "+RTINQ=";                     // start symble when there's any return
String slaveName = ";SlaveUnique";                // caution that ';'must be included, and make sure the slave name is right.


int nameIndex = 0;
int addrIndex = 0;

String recvBuf;
String slaveAddr;


String connectCmd = "\r\n+CONN=";

void setup() {
  Serial.begin(9600);
  setupBlueToothConnection();
  delay(1000);
  Serial.flush();
  Serial2.flush();
}

void loop() {
   char recvChar;
    while(1)
    {
        if(Serial.available())
        {//check if there's any data sent from the local serial terminal, you can add the other applications here
          recvChar  = Serial.read();
          Serial2.write(recvChar);
          Serial.println(recvChar);
        }

        if (Serial2.available()) {       //check if there's any data sent from the remote bluetooth shield
          recvChar = Serial2.read();
          Serial.print(recvChar);
        }
    }
}

void setupBlueToothConnection()
{
    Serial2.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
    Serial2.print("\r\n+STWMOD=1\r\n");             // set the bluetooth work in master mode
    Serial2.print("\r\n+STNA=MasterUnique\r\n");    // set the bluetooth name as "SomethingUnique"
    Serial2.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    delay(2000);                                            // This delay is required.
    Serial2.print("\r\n+INQ=1\r\n");                // make the master bluetooth inquirable
    Serial.println("The master bluetooth is inquiring!");
    delay(2000);                                            // This delay is required.
    Serial2.flush();
     //find the target slave
    char recvChar;
    while(1)
    {
        if(Serial2.available())
        {
            recvChar = Serial2.read();
            recvBuf += recvChar;
            nameIndex = recvBuf.indexOf(slaveName);             //get the position of slave name
          
                                                                //nameIndex -= 1;
                                                                //decrease the ';' in front of the slave name, to get the position of the end of the slave address
            if ( nameIndex != -1 )
            {
                //Serial.print(recvBuf);
                addrIndex = (recvBuf.indexOf(retSymb,(nameIndex - retSymb.length()- 18) ) + retSymb.length());//get the start position of slave address
                slaveAddr = recvBuf.substring(addrIndex, nameIndex);//get the string of slave address
                break;
            }
        }
    }
    
    //form the full connection command
    connectCmd += slaveAddr;
    connectCmd += "\r\n";
    int connectOK = 0;
    Serial.print("Connecting to slave:");
    Serial.print(slaveAddr);
    Serial.println(slaveName);
    //connecting the slave till they are connected
    do
    {
        Serial2.print(connectCmd);//send connection command
        recvBuf = "";
        while(1)
        {
            if(Serial2.available()){
                recvChar = Serial2.read();
                recvBuf += recvChar;
                if(recvBuf.indexOf("CONNECT:OK") != -1)
                {
                    connectOK = 1;
                    Serial.println("Connected!");
                    Serial2.print("Connected!");
                    break;
                }
                else if(recvBuf.indexOf("CONNECT:FAIL") != -1)
                {
                    Serial.println("Connect again!");
                    break;
                }
            }
        }
    }while(0 == connectOK);

  Serial2.flush();
  delay(500);
  while(Serial2.available()) {
    Serial.write(Serial2.read());
  }
}
