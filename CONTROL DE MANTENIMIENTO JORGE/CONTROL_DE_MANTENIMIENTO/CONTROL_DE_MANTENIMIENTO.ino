/* -----------------------------------------------------------------------------------------
               MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
               Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
   Signal      Pin          Pin           Pin       Pin        Pin              Pin
   -----------------------------------------------------------------------------------------
   RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
   SPI SS      SDA(SS)      10            53        D10        10               10
   SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
   SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
   SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
   */
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Ethernet.h>
#include <Keypad.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include "Funtion.h"
#include <SD.h>

   // Enter a MAC address and IP address for your controller below.
   // The IP address will be dependent on your local network:
byte mac[] ={
    0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);
EthernetClient client;

#define SS_PIN 53
#define RST_PIN 19

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.

#define I2C_ADDR 0x27
#define BACKLIGHT_PIN 3
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7
LiquidCrystal_I2C pantalla(I2C_ADDR, En_pin, Rw_pin, Rs_pin, D4_pin, D5_pin, D6_pin, D7_pin);

const byte FILA = 4;    // CUATRO FILAS
const byte COLUMNA = 4; // TRES COLUMNAS
char MATRIZ[FILA][COLUMNA] ={
    { '1', '2', '3', 'A' },
    { '4', '5', '6', 'B' },
    { '7', '8', '9', 'C' },
    { '*', '0', '#', 'D' } };

byte ArregloColumna[COLUMNA] ={ 6, 7, 8, 9 }; // PINES QUE CORRESPONDEN A LAS COLUMNAS DEL TECLADO MATRICIAL
byte ArregloFila[FILA] ={ 2, 3, 4, 5 };       // PINES QUE CORRESPONDEN A LAS FILAS DEL TECLADO MATRICIAL

Keypad keypad = Keypad(makeKeymap(MATRIZ), ArregloFila, ArregloColumna, FILA, COLUMNA);

tmElements_t tm;

File myFile;

String tagLeido = "";       // SE ALMACENA EL TAG QUE SE  ESCANEA
String ordenTrabajo = "";   // SE ALMACENA LAS ORDENES DE TRABAJO
String cedula = "";         // SE ALMACENA LA CEDULA DE LOS USUARIOS
String tagIngresado[10];    // SE ALMACENA TODOS LOS DATOS DE LOS TAG LEIDO
String usuarioIngresar[10]; // SE ALMACENA LOS USUAROS CON TODOS LOS DATOS
String hora = "";
String codigoMaquina = "";
String horaInicial = "";
String horaFinal = "";
String datosUsers = "";
String tabla;

size_t sizeFile;

String HTTP_req = "";

byte ctrlIngreso = 0;
byte controlTagIngresado = 0;
byte ingresoSalida = 0;
byte indUsers = 0;
byte identiUsers = 0;

bool tagControl = false;
bool controlPrimario = false;
bool controlPrincipal = false;

void setup()
{
    Serial.begin(9600);      // Initiate a serial communication
    SPI.begin();             // Initiate  SPI bus
    mfrc522.PCD_Init();      // Initiate MFRC522
    Ethernet.begin(mac, ip); // start the Ethernet connection and the server:
    pantalla.begin(16, 2);
    pantalla.home();
    pantalla.setBacklightPin(BACKLIGHT_PIN, POSITIVE);
    pantalla.setBacklight(HIGH);

    Serial.print("Initializing SD card...");

    if (!SD.begin(4))
    {
        Serial.println("initialization failed!");
        pantalla.print("init SD fallida!");
    }
    else
    {
        Serial.println("initialization done.");
        pantalla.print("init SD exitosa!");
    }

    SD.mkdir("DataUser/");

    if (!SD.exists("DataUser/Userdata.csv"))
    {
        myFile = SD.open("DataUser/Userdata.csv", FILE_WRITE);
        if (myFile)
        {
            myFile.print("TAG; CODIGO DE MAQUINA; OREDEN DE TRABAJO; CEDULA; HORA DE INICIO; HORA TERMINADA \n");
            // close the file:
            myFile.close();
        }
        else
        {
            // if the file didn't open, print an error:
            Serial.println("error abriendo archivo");
        }
    }
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
        Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
        while (true)
        {
            delay(1); // do nothing, no point running without Ethernet hardware
        }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
        Serial.println("Ethernet cable is not connected.");
    }

    // start the server
    server.begin();
    Serial.print("server is at ");
    delay(1000);
    pantalla.clear();
    pantalla.print(Ethernet.localIP());
    Serial.println(Ethernet.localIP());
    delay(1500);
}
void loop()
{
    char key = keypad.getKey();

    if (key)
    {
        Serial.println(key);

        if (key == 'A' && controlPrincipal == false && controlPrimario == false)
        {
            controlPrimario = true;
            controlPrincipal = true;
            pantalla.clear();
            pantalla.setCursor(0, 0);
            pantalla.print("Sistema");
            pantalla.setCursor(0, 1);
            pantalla.print("desbloqueado");
            delay(2000);
            pantalla.clear();
            pantalla.setCursor(0, 0);
            pantalla.print("(*) Ingresar");
            pantalla.setCursor(0, 1);
            pantalla.print("(#) Salir");
        }
        else if (key == 'B' && controlPrincipal == true)
        {
            controlPrincipal = false;
            controlPrimario = false;
            tagControl = false;
            pantalla.clear();
            pantalla.setCursor(1, 0);
            pantalla.print("Sitma bloqueado");
            delay(2000);
            pantalla.clear();
            cedula = "";
            ordenTrabajo = "";
            ingresoSalida = 0;
        }
        else if (key == 'C' && tagControl == false && ingresoSalida == 1 || ingresoSalida == 2)
        {
            pantalla.clear();
            pantalla.setCursor(0, 0);
            pantalla.print("(*) Ingresar");
            pantalla.setCursor(0, 1);
            pantalla.print("(#) Salir");
            ingresoSalida = 0;
            tagControl = true;
            controlPrimario = true;
        }

        else if (controlPrimario == true)
        {
            if (key == '*')
            {
                controlPrincipal = true;
                controlPrimario = false;
                tagControl = false;
                ingresoSalida = 1;
                pantalla.clear();
                pantalla.setCursor(0, 0);
                pantalla.print("Coloca tarjeta");
            }
            else if (key == '#')
            {
                controlPrincipal = true;
                controlPrimario = false;
                tagControl = false;
                ingresoSalida = 2;
                pantalla.clear();
                pantalla.setCursor(0, 0);
                pantalla.print("Coloca tarjeta");
            }
        }
        else if (tagControl == true)
        {
            if (ctrlIngreso <= 2)
            {
                if (key == 'D')
                {
                    if (ctrlIngreso == 0 && ordenTrabajo != "")
                    {
                        pantalla.clear();
                        pantalla.print("Orden guardada..");
                        pantalla.setCursor(0, 1);
                        pantalla.print(ordenTrabajo);
                        delay(2000);
                        pantalla.clear();
                        pantalla.print("Ingresa Cedula.");
                        delay(2000);
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("(D) Ok - (C) Clr");
                        pantalla.setCursor(0, 1);
                        ctrlIngreso = 1;
                    }
                    else if (ctrlIngreso == 1 && cedula != "" && indUsers <= 9)
                    {
                        RTC.read(tm);
                        horaInicial = getHora(tm.Hour, tm.Minute);
                        tagIngresado[controlTagIngresado] = tagLeido;
                        controlTagIngresado++;
                        usuarioIngresar[indUsers] = tagLeido + ";" + codigoMaquina + ";" + ordenTrabajo + ";" + cedula + ";" + horaInicial + ";";
                        indUsers++;
                        pantalla.clear();
                        pantalla.print("Cedula guardada.");
                        pantalla.setCursor(0, 1);
                        pantalla.print(cedula);
                        delay(2000);
                        tagControl = false;
                        ctrlIngreso = 0;
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("(*) Ingresar");
                        pantalla.setCursor(0, 1);
                        pantalla.print("(#) Salir");
                        controlPrimario = true;
                        cedula = "";
                        ordenTrabajo = "";
                        codigoMaquina = "";
                        tagLeido = "";
                    }
                    else if (ctrlIngreso == 2 && codigoMaquina != "")
                    {
                        pantalla.clear();
                        pantalla.print("Codigo guardado.");
                        pantalla.setCursor(0, 1);
                        pantalla.print(codigoMaquina);
                        delay(2000);
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("Orden de trabajo:");
                        delay(2000);
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("(D) Ok - (C) Clr");
                        pantalla.setCursor(0, 1);
                        ctrlIngreso = 0;
                    }
                }
                else if (key == 'C')
                {
                    if (ctrlIngreso == 0)
                    {
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("Orden de trabajo:");
                        delay(2000);
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("(D) Ok - (C) Clr");
                        pantalla.setCursor(0, 1);
                        ordenTrabajo = "";
                    }
                    else if (ctrlIngreso == 1)
                    {
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("Ingresa Cedula:");
                        delay(2000);
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("(D) Ok - (C) Clr");
                        pantalla.setCursor(0, 1);
                        cedula = "";
                    }
                    else if (ctrlIngreso == 2)
                    {
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("Codigo maquina:");
                        delay(2000);
                        pantalla.clear();
                        pantalla.setCursor(0, 0);
                        pantalla.print("(D) Ok - (C) Clr");
                        pantalla.setCursor(0, 1);
                        codigoMaquina = "";
                    }
                }
                if (datoCorrecto(key))
                {
                    if (ctrlIngreso == 0)
                    {
                        pantalla.print(key);
                        ordenTrabajo.concat(key);
                    }
                    else if (ctrlIngreso == 1)
                    {
                        pantalla.print(key);
                        cedula.concat(key);
                    }
                    else if (ctrlIngreso == 2)
                    {
                        pantalla.print(key);
                        codigoMaquina.concat(key);
                    }
                }
            }
        }
    }
    else if (controlPrincipal == true && tagControl == false)
    {
        // Look for new cards
        if (!mfrc522.PICC_IsNewCardPresent())
        {
            return;
        }
        // Select one of the cards
        else if (!mfrc522.PICC_ReadCardSerial())
        {
            return;
        }
        //Show UID on serial monitor
        Serial.print("UID tag :");
        String content = "";
        for (byte i = 0; i < mfrc522.uid.size; i++)
        {
            // Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
            // Serial.print(mfrc522.uid.uidByte[i], HEX);
            content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
            content.concat(String(mfrc522.uid.uidByte[i], HEX));
        }

        mfrc522.PICC_HaltA(); // Stop reading
        content.toUpperCase();
        tagLeido = content.substring(1);
        Serial.println(tagLeido);

        if (tagLeido != "" && controlTagIngresado <= 9)
        {
            if (ingresoSalida == 1)
            {
                if (tagLeido == verifTags(tagIngresado, tagLeido, &identiUsers))
                {
                    pantalla.clear();
                    pantalla.home();
                    pantalla.print("Usuario ya fue ");
                    pantalla.setCursor(0, 1);
                    pantalla.print("fue ingresado");
                    delay(1500);
                    pantalla.clear();
                    pantalla.setCursor(0, 0);
                    pantalla.print("Coloca tarjeta");
                }
                else
                {
                    pantalla.clear();
                    pantalla.setCursor(0, 0);
                    pantalla.print("Codigo maquina:");
                    delay(2000);
                    pantalla.clear();
                    pantalla.setCursor(0, 0);
                    pantalla.print("(D) Ok - (C) Clr");
                    pantalla.setCursor(0, 1);
                    tagControl = true;
                    ingresoSalida = 0;
                    ctrlIngreso = 2;
                }
            }
            else if (ingresoSalida == 2)
            {
                if (SD.begin(4))
                {
                    //myFile.seek(0);
                    if (SD.exists("DataUser/Userdata.csv"))
                    {
                        myFile = SD.open("DataUser/Userdata.csv", FILE_WRITE);
                        if (myFile)
                        {
                            if (tagLeido == verifTags(tagIngresado, tagLeido, &identiUsers))
                            {
                                RTC.read(tm);
                                horaFinal = getHora(tm.Hour, tm.Minute);
                                usuarioIngresar[identiUsers].concat(horaFinal);
                                myFile.println(usuarioIngresar[identiUsers]);
                                Serial.println("indice = " + String(identiUsers));
                                Serial.println(usuarioIngresar[identiUsers]);
                                Serial.println("\nindice = " + String(identiUsers));
                                borrarTU(identiUsers, tagIngresado, usuarioIngresar, tagLeido, &controlTagIngresado, &indUsers);
                                orgDatos(usuarioIngresar, tagIngresado);
                                for (byte i = 0; i <= 9; i++)
                                {
                                    Serial.println("Usuarios = " + usuarioIngresar[i]);
                                    Serial.println("Tags = " + tagIngresado[i]);
                                }
                                pantalla.clear();
                                pantalla.home();
                                pantalla.print("Salida exitosa");
                                pantalla.setCursor(0, 1);
                                pantalla.print(horaFinal);
                                delay(1000);
                                pantalla.clear();
                                pantalla.setCursor(0, 0);
                                pantalla.print("(*) Ingresar");
                                pantalla.setCursor(0, 1);
                                pantalla.print("(#) Salir");
                                controlPrimario = true;
                                ingresoSalida = 0;
                                Serial.println("\nindice de tags almacenado = " + String(controlTagIngresado) + "\tindice de usuarios almacenado = " + String(indUsers));
                            }
                            else
                            {
                                pantalla.clear();
                                pantalla.home();
                                pantalla.print("Buscando Tag...");
                                delay(1000);
                                pantalla.clear();
                                pantalla.home();
                                pantalla.print("Tag no ingresado");
                                delay(1000);
                                pantalla.clear();
                                pantalla.setCursor(0, 0);
                                pantalla.print("(*) Ingresar");
                                pantalla.setCursor(0, 1);
                                pantalla.print("(#) Salir");
                                controlPrimario = true;
                                ingresoSalida = 0;
                            }
                            myFile.close();
                        }
                        else
                        {
                            Serial.println("error abriendo el archivo");
                            pantalla.clear();
                            pantalla.print("error open file");
                        }
                    }
                    else
                    {
                        Serial.println("archivo no existe");
                        pantalla.clear();
                        pantalla.print("archivo no exist");
                    }
                }
                else
                {
                    Serial.println("No SD");
                    pantalla.clear();
                    pantalla.print("SD retirada :(");
                }
            }
        }
        else
        {
            pantalla.clear();
            pantalla.print("Excede # users");
        }
    }
    else if (controlPrimario == false && controlPrincipal == false)
    {
        pantalla.setCursor(0, 0);
        pantalla.print("(A) desbloquear");
        pantalla.setCursor(0, 1);
        pantalla.print("(B) bloquear");

        // listen for incoming clients
        client = server.available();
        if (client)
        {
            Serial.println("new client");
            // an http request ends with a blank line
            boolean currentLineIsBlank = true;
            boolean contDown = false;

            while (client.connected())
            {
                if (client.available())
                {

                    char c = client.read();
                    Serial.write(c);
                    HTTP_req += c; // save the HTTP  1 char at a time

                    if (HTTP_req.indexOf("/V") > 0 || HTTP_req.indexOf("/D") > 0 || HTTP_req.indexOf("/O") > 0)
                    {
                        if (HTTP_req.indexOf("/V") > 0)
                        {
                            if (SD.begin(4))
                            {
                                if (SD.exists("DataUser/Userdata.csv"))
                                {
                                    myFile = SD.open("DataUser/Userdata.csv");
                                    if (myFile)
                                    {
                                        size_t iter = 0;
                                        sizeFile = myFile.size();
                                        sizeFile = sizeFile - 1;
                                        datosUsers = "";
                                        tabla = "";
                                        tabla = mostrarTabla(myFile, sizeFile);
                                        Serial.println(tabla);
                                        datosUsers = mostrarDatosCsv(myFile, sizeFile);
                                        Serial.println(datosUsers);
                                        contDown = true;
                                    }
                                    else
                                    {
                                        Serial.println("Error abirendo el archivo");
                                        tabla = "Error abirendo el archivo";
                                    }
                                }
                                else
                                {
                                    Serial.println("Fichero o archivo que se intenta abrir no existe");
                                    tabla = "Fichero o archivo que se intenta abrir no existe";
                                }
                                myFile.close();
                            }
                            else
                            {
                                Serial.println("Error al iniciar SD");
                                tabla = "Error al iniciar SD";
                            }
                        }
                        if (HTTP_req.indexOf("/D") > 0)
                        {
                            client.println(datosUsers);
                            client.stop();
                            client.flush();
                        }
                        if (HTTP_req.indexOf("/O") > 0)
                        {
                            datosUsers = "";
                            tabla = "";
                            contDown = false;
                        }
                        delay(500);
                        HTTP_req = "";
                    }

                    if (c == '\n' && currentLineIsBlank)
                    {
                        // send a stan
                        //output HTML data header
                        client.println(F("HTTP/1.1 200 OK"));
                        client.println(F("Content-Type: text/html"));
                        client.println();
                        //header
                        client.print(F("<!DOCTYPE HTML><html><head><title>Gestion de usuarios</title>"));
                        client.print(F("<meta http-equiv='content-type' content='text/html; charset=UTF-8'>"));
                        //meta-refresh page every x seconds

                        client.print(F("</head><body bgcolor='rgb(7, 15, 20)'><br>"));
                        client.print(F("<hr/><hr>"));
                        client.print(F("<h1 style='color : #3AAA35;'><center>REPORTE DE TIEMPO DE RECORRIDO DE MANTENIMIENTO</center></h1>"));
                        client.print(F("<hr/><hr>"));
                        client.println("<center><p style='color:white;'>");

                        client.println("</p></center><br>");
                        client.print(F("<a href='/V'><button>  Ver registros  -----</button></a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp"));
                        client.print(F("<a href='/O'><button>Ocultar registros</button></a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp"));
                        if (contDown == true)
                        {
                            client.print(F("<a href='/D'download = 'Userdata.csv'><button>Descargar reporte</button></a>"));
                        }

                        client.print(F("<br><br><br>"));
                        client.print(F("<p style='color:white;'>"));

                        client.print(tabla);
                        client.print(F("<br><br><br>"));

                        //file end
                        client.print(F("</body></html>"));
                        break;
                    }
                    if (c == '\n')
                    {
                        // you're starting a new line
                        currentLineIsBlank = true;
                    }
                    else if (c != '\r')
                    {
                        // you've gotten a character on the current line
                        currentLineIsBlank = false;
                    }
                }
            }
            // give the web browser time to receive the data
            delay(1);
            // close the connection:
            client.stop();
            client.flush();
            Serial.println("client disconnected");
        }
    }
}
