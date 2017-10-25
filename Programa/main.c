#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define LONG_MAX_LINEA  1024
#define MAX_ARGS 90

#define etapa1 "etapa1.txt"
#define etapa2 "etapa2.txt"
#define etapa3 "etapa3.txt"

int id_layer = 1; // contendra el id de cada layer

/// estructura a utilizar para guardar la informacion extraida de la pagina xml
struct layer
{
    int idLayer;
    char* title;
    char* abstract;
    int eliminado; // 1 es falso y 0 es true
    struct layer *sig;
} *cabeza;

/// funcion encargada de la creacion de un nuevo nodo tipo layer que almacenara la informacion requerida, title, abstract
/// insercion al inicio
/// title, linea 1 de abstract
void insertarLayer(char* t, char* ab1)
{
    struct layer *newLayer = (struct layer*) malloc(sizeof(struct layer));

    newLayer->idLayer = id_layer;
    newLayer->title = t;
    newLayer->abstract = ab1;
    newLayer->sig = NULL;
    newLayer->eliminado = 1; // 1 quiere decir falso

    id_layer++;
    if (cabeza == NULL)
        cabeza = newLayer;
    else
    {
        newLayer->sig = cabeza;
        cabeza = newLayer;
    }
}

/// funcion encargada de mostrar la lista de capas que se tienen para guardar
void mostrarLista()
{
    struct layer *temp = cabeza;

    while(temp != NULL)
    {
        if (temp->eliminado == 1) /// sino esta marcado como eliminado lo muestras
            printf("\n%d. %s",temp->idLayer,temp->title);
        temp = temp->sig;
    }
}

/// funcion que cambia el valor del campo eliminado (dentro del nodo tipo layer que coincida con el id pasado) para no tomarlo en cuenta a la hora
/// del guardado de la informacion
int eliminarElemento(int id)
{
    struct layer *temp = cabeza;

    while(temp != NULL)
    {
        if(temp->idLayer == id) /// nodo que se desea eliminar
        {
            if(temp->eliminado == 1) /// 1 falso (si NO esta eliminado), 0 si esta en opcion para guardar
            {
                temp->eliminado = 0;
                printf(">> \nElemento #%d eliminado.\n",id);
            }
            else
                printf("\n>> \nEl elemento que quiere eliminar ya se encuentra eliminado.\n");
            return 0;
        }
        temp = temp->sig;
    }
    return 1;
}

/// funcion encargada de retornar un substring de la cadena enviada por parametros
/// cadena enviada de la cual se va a extraer cierta cantidad de caracteres, posicion de la cadena en la que se empieza a "cortar" la cadena final,
/// cantidad a leer (final de lectura), si se pone 0 entonces se toma TODA la cadena desde la posicion de la variable comienzo hasta el final de la cadena
char* substr(char* cadena, int comienzo, int longitud)
{
    if (longitud == 0)
        longitud = strlen(cadena)-comienzo;

    char *nuevo = (char*) malloc(sizeof(char) * (longitud+1));
    nuevo[longitud] = NULL;
    strncpy(nuevo, cadena + comienzo, longitud);

    return nuevo;
}

/// funcion encargada del inicio de sesion y creacion del primer archivo xml que contiene la ventana principal.
void crearArchivo(char* URL, char* name)
{
    CURL *curl;
    CURLcode res;

    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL,URL);

        FILE* file = fopen( name, "w");

        curl_easy_setopt( curl, CURLOPT_WRITEDATA, file) ;

        /* Realiza la solicitud, despues obtendrá el código de retorno */
        res = curl_easy_perform(curl);

        /* Comprueba si hay errores */
        if(res != CURLE_OK)
          fprintf(stderr, "curl_easy_perform() failed: %s\n",
                  curl_easy_strerror(res));

        /* siempre limpiar */
        curl_easy_cleanup(curl);

        fclose(file);
    }
    //system("clear"); // limpia la consola
}

/// funcion encargada de la entraccion de varios substring y devuelve un int con la cantidad de subcadenas creadas a partir del delimitador
/// linea leida del archivo, delimitador para hacer el split, lista donde se almacenaran las subcadenas, cantidad maxima de argumentos
int subStr(char *origen, char *delimitador, char *args[], int max_args)
{
    char *temp;
    int num=0;
    /* Reservamos memoria para copiar la candena ... pero la memoria justa */
    char *str = malloc(strlen(origen)+1);
    strcpy(str, origen);

    /* Extraemos la primera palabra */
    temp=strtok(str, delimitador);

    do
    {
      if (num==max_args)
        return max_args+1;  /* Si hemos extraído más cadenas que palabras devolvemos el número de palabras máximo y salimos */

      args[num]=temp;            /* Copiamos la dirección de memoria tmp en args[num] */
      num++;

      /* Extraemos la siguiente palabra */
      temp = strtok(NULL, delimitador);
    } while (temp != NULL);

    return num;
}

/// funcion principal encargada de leer y crear archivos dependiendo de la etapa
void obtenerDatos()
{
    FILE * file = fopen (etapa1, "r");/// declaracion de archivo
    char linea[LONG_MAX_LINEA], lineaSecundaria[LONG_MAX_LINEA];

    char URL[13];
    char* palabras[MAX_ARGS];
    char* title; // almacena el title
    char* abstract1; // almacena primer linea del abstract
    int cantArgumentos = 0, cantAtoms = 0;

    if (file == NULL)
    {
        printf("\n>> Error al abrir el archivo %s, asegurese de tener internet.\n",etapa1);
        exit(1);
    }
    while (fgets(linea, LONG_MAX_LINEA, file) != NULL) /// while para recorrer archivo de primer etapa (nivel principal)
    {
        /// si encuentro el atom procedo a buscar como recuperar la URL
        if (strstr(linea,"<atom")!=NULL)
        {
            cantArgumentos = subStr(linea, "\"", palabras, MAX_ARGS); /// hace un split y devuelve la cantidad de letras generadas

            for(int i = 0; i < cantArgumentos; i++) /// recorremos buscando la palabra que CONTENGA la frase "http://www.idehn" y esa es la url valida a usar
            {
                if(strstr(palabras[i],"http://www.idehn") != NULL) /// buscamos la palabra que tenga nuestra url
                {
                    FILE * fileCapa2;
                    char temp [strlen(palabras[i])]; /// creacion de char para darle forma a la url (darle permisos de acceso)
                    strncpy(temp,"test1:test1@",13); /// permisos de acceso

                    strncat(temp,substr(palabras[i],7,0),strlen(palabras[i])); /// url terminada, con permisos de acceso y la url juntos, URL lista para usar

                    crearArchivo(temp,etapa2); /// se crea el archivo de la capa 2 usando la url generada

                    fileCapa2 = fopen (etapa2, "r"); /// abrimos el archivo de capa 2 para extrae el SEGUNDO atom de este nivel

                    if (fileCapa2 == NULL)
                    {
                        printf("\n >>Error al abrir el archivo 2\n");
                        exit(1);
                    }
                    while(fgets(lineaSecundaria, LONG_MAX_LINEA, fileCapa2) != NULL) /// while para recorrer archivo de segunda etapa
                    {
                        if(strstr(lineaSecundaria,"<atom")!=NULL) /// si detecta atom trataremos de sacar la url a la siguiente etapa
                        {
                            cantAtoms++;
                            if(cantAtoms == 2)/// el segundo atom es el que en realidad importa porque tiene la url a la tercer etapa
                            {
                                cantAtoms = 0;
                                cantArgumentos = subStr(lineaSecundaria, "\"", palabras, MAX_ARGS);

                                for(int i = 0; i < cantArgumentos; i++)
                                {
                                    if(strstr(palabras[i],"http://www.idehn") != NULL)
                                    {
                                        char temp [strlen(palabras[i])];
                                        strncpy(temp,"test1:test1@",13);

                                        strncat(temp,substr(palabras[i],7,0),strlen(palabras[i])); /// url lista

                                        crearArchivo(temp,etapa3); /// se crea el archivo de la capa 3

                                        fileCapa2 = fopen (etapa3, "r"); ///abrimos el archivo 3 el cual contiene la ultima capa con el title y el abstract
                                        if (fileCapa2 == NULL)
                                        {
                                            printf("\n>> Error al abrir el archivo 3\n");
                                            exit(1);
                                        }
                                        cantAtoms = 0; // limpiamos atom por si acaso
                                        while(fgets(lineaSecundaria, LONG_MAX_LINEA, fileCapa2) != NULL) /// while para recorrer archivo de tercer etapa
                                        {
                                            if(strstr(lineaSecundaria,"<title>") != NULL) /// si está la etiqueta title
                                                title = strtok(substr(lineaSecundaria,9,0),"<"); /// obtiene el title strtok realiza un split enviandole la cadena y el delimitador

                                            else if(strstr(lineaSecundaria,"<abstract>") != NULL) /// si está la etiqueta abstract
                                            {
                                                if(!strstr(lineaSecundaria,"</abstract>") != NULL) /// si la linea leída NO tiene el cierre de la etiqueta abstract
                                                    abstract1 = substr(lineaSecundaria,12,0);

                                                else /// si la linea SI tiene el cierre de la etiqueta abstract
                                                    abstract1 = strtok(substr(lineaSecundaria,12,0),"<");

                                                insertarLayer(title,abstract1);
                                                break; /// termina ciclo while de tercer nivel
                                            }// cierre else
                                        } // cierre while
                                        break; /// cierra ciclo for de recorrido de palabras
                                    }//cierre if
                                }// cierre for
                                break; /// cierra ciclo de recorrido de archivo segundo nivel
                            } // cierre if
                        }// cierre de if
                    } // cierre de while
                    break;/// cierra ciclo for de palabras
                }// cierre de if sacado de la url nivel1->nivel2
            }// cierre de for nivel1
        }// cierre de if nivel
    }// cierre de while nivel1
}

/// funcion que guarda TODA la informacion almcenada en la estructura layer
void guardarTODO()
{
    struct layer *temp = cabeza;
    FILE* file = fopen("data.txt","w");
    while(temp != NULL){
        fprintf(file,"\n%d. %s",temp->idLayer,temp->title);
        fprintf(file,"\n\tDescripción: %s",temp->abstract);
        temp = temp->sig;
    }
    fclose(file);
}

/// funcion que guarda las capas seleccionadas de la lista a guardar
void guardarSeleccion()
{
    struct layer *temp = cabeza;
    FILE* file = fopen("data.txt","w");
    while(temp != NULL){
        if(temp->eliminado == 1)
        {
            fprintf(file,"\n%d. %s",temp->idLayer,temp->title);
            fprintf(file,"\n\tDescripción: %s",temp->abstract);
        }
        temp = temp->sig;
    }
    fclose(file);
}

/// funcion que pregunta que preguntas desea eliminar de la lista a guardar
void pregBorrar()
{
    int resp = 0;

    char sNum[4];
    int numId;

    char sOp[1];
    int  op = 0;

    while(resp == 0)// while true
    {
        mostrarLista();
        printf("\n\n* * *\nNOTA: si ingresa un numero seguido de letras se toma el numero como el id de un elemento EJEMPLO: 10puravida se toma el numero 10 y se elimina.\nAdemás si ingresa un dato mal se le indica el error arriba (debe subir el scroll)\n* * *\n\nDigite el número del elemento a eliminar: \n");

        scanf("%s",sNum);
        numId = atoi(sNum); /// atoi recibe un char y devuelve 0 si ingreso letras y si ingreso un numero devuelve el numero ingresado

        if(numId == 0) /// sino se logra la asignacion quiere decir que metio una letra por lo que se muetra el error
        {
            system("clear");
            printf("\n>> Error en dato ingresado, debe ser el número de un elemento.\n");
        }
        else{
            if (eliminarElemento(numId) == 1) /// si retorna 1 no lo encontro, caso contrario lo encontro y lo elimino
            {
                system("clear");
                printf("\n>> El id insertado no coincide con ninguno de la lista.\n");
                continue;
            }
            system("clear");
            printf("\n\n¿Desea continuar eliminando elementos de la lista a guardar?  \n\t1. Si\n\t2. No\n\t");

            scanf("%s",sOp);
            op = atoi(sOp);

            if (op == 0) /// continuar eliminando
            {
                system("clear");
                printf("\n>> Error en dato ingresado, debe ser una de las opciones, 1 (si desea continuar eliminando elementos) o 2 (si desea dejar de eliminar elementos).\n");
            }
            else if(op == 1) /// salir de la eliminacion de elementos
                resp = 0;
            else if(op == 2) /// salir de la eliminacion de elementos
                resp = 1;
            else{
                system("clear");
                printf("\n>> Error al seleccionar al opción, el programa se detendrá.\n");
            }
        }
    }
}

/// funcion encargada de mostrar el contenido del archivo data.txt
void mostrarArchivo()
{
    FILE *file = fopen("data.txt","r");
    char linea[LONG_MAX_LINEA];
    while(fgets(linea, LONG_MAX_LINEA, file) != NULL)
        printf("%s",linea);

    fclose(file);
}

/// el main puede o no recibir parametros
/// en argc se guarda la cantidad de parametros enviados (el nombre del proyecto es uno)
/// el argv guarda los parametros en posiciones de ese vector
int main(int argc, char *argv[])
{
    if(argc > 3)
    {
        printf("\n>> Error, solamente pueden haber 3 parametros, no más. Pura vida!\n");
        exit(1);
    }
    char op [3];

    system("clear");

    crearArchivo("http://test1:test1@www.idehn.tec.ac.cr/geoserver/rest/layers.xml",etapa1); /// crea el archivo para almacenar el nivel 1 para poder recorrer

    printf("\nObteniendo información...\n");

    obtenerDatos(); ///  guarda los datos requeridos en una estructura recorriendo los archivos en los que se guardaron cada una de las paginas

    if (argc == 1)/// solo ingreso el nombre del archivo PREGUNTO QUE DESEA ELIMINAR Y QUE DESEA GUARDAR
    {
        pregBorrar();
        guardarSeleccion();
    }
    else if (argc == 2)/// ingreso nombre y un parametro que deberia ser -f GUARDA TODO
    {
        if (strcmp(argv[1], "-f") == 0) /// si devuelve 0 quiere decir que el parametro ingresado es igual
        {
            guardarTODO();
            system("clear");
            printf("\nDatos almcenados correctamente en un archivo llamado data.txt.\n");
        }
        else if (strcmp(argv[1], "-p") == 0)
        {

            pregBorrar();
            guardarSeleccion();
            printf("\n\n============ MOSTRAR POR PARAMETRO -p  ============\n");
            mostrarArchivo();
            exit(1);
        }
        else
        {
            printf("\n>> Error, el segundo parametro debe ser -f o -p, pura vida!\n");
            exit(1);
        }
    }
    else if (argc == 3)
    {
        if ((strcmp(argv[1], "-f") == 0) & (strcmp(argv[2], "-p") == 0))
        {
            guardarTODO();
            system("clear");
            printf("\nDatos almcenados correctamente en un archivo llamado data.txt.\n");
            printf("\n\n============ MOSTRAR POR PARAMETRO -p  ============\n");
            mostrarArchivo();
        }
        else
        {
            printf("\n>> Error, el segundo parametro debe ser -f y el tercero -p, pura vida!\n");
            exit(1);
        }
    }
    else
    {
        printf("\n>> Error, solamente debe ingresar un parametros además del nombre, el cual es -f para guardar todas las capas con su descripción, pura vida!\n");
        exit(1);
    }
    return 0;
}

