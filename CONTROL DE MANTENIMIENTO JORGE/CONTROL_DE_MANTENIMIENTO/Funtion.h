#include <SD.h>
bool datoCorrecto(char key)
{
    bool valor;
    switch (key)
    {
    case 'A':
        valor = false;
        break;
    case 'B':
        valor = false;
        break;
    case 'C':
        valor = false;
        break;
    case 'D':
        valor = false;
        break;
    case '*':
        valor = false;
        break;
    case '#':
        valor = false;
        break;
    default:
        valor = true;
        break;
    }
    return valor;
}

String getHora(int number, int number2)
{
    String hora = "0";
    String minuto = "0";
    if (number >= 0 && number < 10)
    {
        hora.concat(String(number));
    }
    else
    {
        hora = String(number);
    }
    if (number2 >= 0 && number2 < 10)
    {
        minuto.concat(String(number2));
    }
    else
    {
        minuto = String(number2);
    }
    return hora + ":" + minuto;
}

String getDate(int dia, int mes, String ano)
{
    return String(dia) + "/" + String(mes) + "/" + ano;
}

void mostrarTable(File myFile, EthernetClient client, size_t sizeFile)
{
    String table = "<table border = '2'style='color:white' style='border: blue'; bgcolor = '#5E332A;'><caption style='color:white'><h3>Reporte de tiempo transcurrido durante el mantenimiento</h3></caption><br><br><tr><th>";
    client.print(table);
    size_t iter = 0;
    char leer;
    boolean sw = true;

    myFile.seek(0);
    while (iter <= sizeFile)
    {
        leer = myFile.read();
        switch (leer)
        {
        case ';':
            if (sw == true)
            {
                client.print(F("</th><th>"));
            }
            else
            {
                client.print(F("</td><td>"));
            }
            break;
        case '\n':
            if (sw == true && iter < sizeFile)
            {
                client.print(F("</th></tr><tr><td>"));
                sw = false;
            }
            else if (iter == sizeFile)
            {
                client.print(F("</td></tr></table>"));
            }
            else
            {
                client.print(F("</td></tr><tr><td>"));
            }
            break;
        default:
            client.print(leer);
            break;
        }
        iter++;
    }
}

void mostrarDatosCSV(File myFile, EthernetClient client, size_t sizeFile)// SE  MUETRAN LOS DATOS PARA SER DESCARGADOS EN EL FICHERO
{
    char leer;
    size_t iter = 0;
    myFile.seek(0);
    while (iter <= sizeFile)
    {
        leer = myFile.read();
        client.print(leer);
        iter++;
    }
}

String verifTags(String tagIngresado[], String tagLeido, byte *identiUsers)
{
    String tagAlmcendo = "";
    bool value = false;

    for (byte i = 0; i <= 9; i++)
    {
        if (tagIngresado[i] == tagLeido)
        {
            tagAlmcendo = tagIngresado[i];
            value = true;
            *identiUsers = i;
        }
    }
    if (value == true)
    {
        return tagAlmcendo;
    }
    else
    {
        return "";
    }
}

void borrarTU(byte identiUsers, String tagIngresado[], String usuarioIngresar[], String tagLeido, byte *controlTagIngresado, byte *indUsers)
{
    if (tagIngresado[identiUsers] == tagLeido)
    {
        tagIngresado[identiUsers] = "";
        usuarioIngresar[identiUsers] = "";
    }
    *indUsers = *indUsers - 1;
    *controlTagIngresado = *controlTagIngresado - 1;
}
void orgDatos(String usuarioIngresar[], String tagIngresado[])
{
    byte control[2] ={ 0, 0 };
    String tag[10];
    String users[10];

    for (byte i = 0; i <= 9; i++)
    {
        if (tagIngresado[i] != "")
        {
            tag[control[1]] = tagIngresado[i];
            control[1]++;
        }
        if (usuarioIngresar[i] != "")
        {
            users[control[0]] = usuarioIngresar[i];
            control[0]++;
        }
    }
    for (byte i = 0; i <= 9; i++)
    {
        tagIngresado[i] = tag[i];
        usuarioIngresar[i] = users[i];
    }
}
