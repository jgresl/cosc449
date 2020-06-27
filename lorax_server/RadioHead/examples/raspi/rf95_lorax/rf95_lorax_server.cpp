#include <RHDatagram.h>
#include <RHReliableDatagram.h>
#include <RH_RF95.h>
#include <bcm2835.h>
#include <json-c/json.h>
#include <mysql.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/* Raspberry Pi wiring configurations */
#define RF95_CHIP_SELECT_PIN RPI_V2_GPIO_P1_22  // Slave Select on GPIO25 so P1 connector pin #22
#define RF95_INTERRUPT_PIN RPI_V2_GPIO_P1_07    // IRQ on GPIO4 so P1 connector pin #7
#define RF95_RESET_PIN RPI_V2_GPIO_P1_11        // Reset on GPIO17 so P1 connector pin #11

/* LoRa radio configurations */
#define RF95_FREQUENCY 915.0          // Between 137.0 and 1020.0   (Default = 915 Mhz)
#define RF95_TRANSMISSION_POWER 20    // Between 5 and 23           (Default = 13 Db)
#define RF95_CAD_TIMEOUT 10000        // Greater or equal to 0      (Default = 0 ms)
#define RF95_SPREADING_FACTOR 12      // Between 6 and 12           (Default = ?)             --> Overwritten by setMOdemConfig()
#define RF95_BANDWIDTH 125000         // Between 7800 and 500000    (Default = 125000 Hz)     --> Overwritten by setMOdemConfig()
#define RF95_PREAMBLE_LENGTH 0x39     //                            (Default = ?)
#define RF95_SYNC_WORD 0x39           //                            (Default = 0x39)
#define RF95_CODING_RATE 5            // Between 5 and 8            (Default = 5-bit)         --> Overwritten by setMOdemConfig()
#define RF95_RETRANSMIT_TIMEOUT 2000  //                            (Default = 200ms)
#define RF95_GATEWAY_ID 100           // Assign unique ID to gateway

/* Declare functions */
void initialize_gateway();
void relay_data();
void print_catalog();
uint8_t get_node_id(char*);
char* get_date_time();
void print_sql_error(MYSQL*);
void update_last_transmission(uint8_t);
void insert_sample(char*);

/* Create instance of the radio driver */
RH_RF95 rf95(RF95_CHIP_SELECT_PIN, RF95_INTERRUPT_PIN);

/* Create instance of the packet manager */
RHReliableDatagram manager(rf95, RF95_GATEWAY_ID);

/* Setup function */
int main(void) {
    initialize_gateway();

    /* Loop function */
    while (1) {
        relay_data();
    }
    return EXIT_SUCCESS;
}

void initialize_gateway() {
    printf("\nRaspberry Pi LoRa Gateway : %s\n", get_date_time());
    printf("----------------------------------------------------------\n");

    /* Initialize BCM */
    printf("Initializing BCM:\t\t");
    while (!bcm2835_init()) {
        printf("Failed\n");
        while (1)
            ;
    }
    printf("Successfull\n");

    /* Initialize LoRa radio */
    printf("Initializing radio:\t\t");
    while (!manager.init()) {
        printf("Failed\n");
        while (1)
            ;
    }
    printf("Successfull\n");

    /* Set frequency (ISM band) */
    printf("Frequency:\t\t\t");
    rf95.setFrequency(RF95_FREQUENCY);
    printf("%f Mhz\n", RF95_FREQUENCY);

    /* Set transmission power (5 to 23 dBm) */
    printf("Transmission power:\t\t");
    rf95.setTxPower(RF95_TRANSMISSION_POWER, false);
    printf("%d dBm\n", RF95_TRANSMISSION_POWER);

    /* Set channel active detection timeout
    printf("CAD timeout:\t\t\t");
    rf95.setCADTimeout(RF95_CAD_TIMEOUT);
    printf("%d ms\n", RF95_CAD_TIMEOUT);  */

    /* Set spreading factor 
    printf("Spreading factor:\t\t");
    rf95.setSpreadingFactor(RF95_SPREADING_FACTOR);
    printf("SF%d\n", RF95_SPREADING_FACTOR); */

    /* Set signal bandwith
    printf("Bandwidth:\t\t\t");
    rf95.setSignalBandwidth(RF95_BANDWIDTH);
    printf("%d Hz\n", RF95_BANDWIDTH); */

    /* Set preamble length 
    printf("Preamble length:\t\t");
    rf95.setSignalBandwidth(RF95_PREAMBLE_LENGTH);
    printf("%x\n", RF95_PREAMBLE_LENGTH); */

    /* Set sync word 
    printf("Sync word:\t\t\t");
    rf95.spiWrite(RH_RF95_REG_39_SYNC_WORD, RF95_SYNC_WORD);
    printf("%x\n", RF95_SYNC_WORD); */

    /* Set coding rate 
    printf("Coding rate:\t\t\t");
    rf95.setCodingRate4(5);
    printf("%d-bit\n", RF95_CODING_RATE); */

    /* Set minimum re-transmit timeout */
    printf("Re-transmit timeout:\t\t");
    manager.setTimeout(RF95_RETRANSMIT_TIMEOUT);
    printf("%d ms\n", RF95_RETRANSMIT_TIMEOUT);

    /* Set modem configuration */
    printf("Modem configuration:\t\t");
    if (!rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128))
        printf("Invalid setModemConfig() option\n");
    printf("%s\n", "Bw500Cr45Sf128");

    printf("----------------------------------------------------------\n\n");
}

void relay_data() {
    if (manager.available()) {
        /* Wait for a message addressed to us from a node */
        uint8_t node_id, incoming_message[150];
        uint8_t incoming_message_length = sizeof(incoming_message);
        if (manager.recvfromAck(incoming_message, &incoming_message_length, &node_id)) {
            printf("Received from node %d:\t\t%s\n", node_id, (char*)incoming_message);
            if (node_id == 0) {
                /* Send request_id and date_time to node */
                char outgoing_message[100];
                sprintf(outgoing_message, "{\"node_id\":\"%u\", \"date_time\":\"%s\"}", get_node_id((char*)incoming_message), get_date_time());
                printf("Sending to node %d:\t\t%s\n", node_id, outgoing_message);
                if (manager.sendtoWait((uint8_t*)outgoing_message, sizeof(outgoing_message), node_id)) {
                    printf("Received from node %d:\t\tTransmission acknowledged\n\n", node_id);
                } else {
                    printf("Received from node %d:\t\t<no acknowledgment>\n\n", node_id);
                }
            } else {
                /* Update last transmission in mysql Node table */
                insert_sample((char*)incoming_message);
                update_last_transmission(node_id);
            }
        }
    }
    return;
}

uint8_t get_node_id(char serial_number[]) {
    char sql[150];
    uint8_t node_id;

    /* Open the database connection */
    MYSQL* conn;
    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, "localhost", "web_user", "web_pass", "lorax", 0, NULL, 0) == NULL) {
        print_sql_error(conn);
    }

    /* Query the database to see if the node serial_number is registered */
    sprintf(sql, "SELECT node_id, serial_number FROM Node WHERE serial_number = '%s'", serial_number);
    if (mysql_query(conn, sql)) {
        print_sql_error(conn);
    }

    /* Store the query result and count number of fields and rows */
    MYSQL_RES* result = mysql_store_result(conn);
    int num_fields = mysql_num_fields(result);
    int num_rows = mysql_num_rows(result);

    /* Check to see if the result set is empty */
    if (result == NULL) {
        print_sql_error(conn);
    } else if (num_rows == 0) {
        sprintf(sql, "INSERT INTO Node (serial_number, last_transmission) VALUES ('%s', '%s')", serial_number, get_date_time());
        mysql_query(conn, sql);
        node_id = mysql_insert_id(conn);
        printf("New node inserted at row %d\n", node_id);
        print_catalog();
    } else if (num_rows > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        sscanf(row[0], "%u", &node_id);
    }

    /* Close the database connection  */
    mysql_free_result(result);
    mysql_close(conn);

    return node_id;
}

void insert_sample(char* incoming_message) {
    char sql[150];

    /* Open the database connection */
    MYSQL* conn;
    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, "localhost", "web_user", "web_pass", "lorax", 0, NULL, 0) == NULL) {
        print_sql_error(conn);
    }

    /* Parse JSON message */
    struct json_object* parsed_json;
    struct json_object* id;
    struct json_object* type;
    struct json_object* time;
    struct json_object* val;
    struct json_object* samp;

    parsed_json = json_tokener_parse(incoming_message);
    json_object_object_get_ex(parsed_json, "node_id", &id);
    json_object_object_get_ex(parsed_json, "sensor_type", &type);
    json_object_object_get_ex(parsed_json, "date_time", &time);
    json_object_object_get_ex(parsed_json, "value", &val);
    json_object_object_get_ex(parsed_json, "sample", &samp);

    /* Insert parsed values into Sample table of LoRaX database */
    int node_id = json_object_get_int(id);
    const char* sensor_type = json_object_get_string(type);
    const char* date_time = json_object_get_string(time);
    int value = json_object_get_int(val);
    int sample = json_object_get_int(samp);
    sprintf(sql, "INSERT INTO Sample (node_ID, sensor_type, date_time, value, sample) VALUES (%d, '%s', '%s', %d, %d)", node_id, sensor_type, date_time, value, sample);

    /* A node_id > 0 indicates the JSON was parsed correctly because node 0 means unsynchronized node and should not be transmitting samples yet) */
    if (node_id > 0) {
        if (mysql_query(conn, sql)) {
            print_sql_error(conn);
        }
    } else {
        printf("\nBad SQL (Skipping insert):\t");
        printf(sql);
        printf("\n\n");
    }

    /* Close the database connection */
    mysql_close(conn);
}

void update_last_transmission(uint8_t node_id) {
    char sql[250];

    /* Open the database connection */
    MYSQL* conn;
    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, "localhost", "web_user", "web_pass", "lorax", 0, NULL, 0) == NULL) {
        print_sql_error(conn);
    }

    /* Query the database to see if the node serial_number is registered */
    sprintf(sql, "UPDATE Node SET last_transmission = '%s' WHERE node_id = %u", get_date_time(), node_id);
    if (mysql_query(conn, sql)) {
        print_sql_error(conn);
    }

    /* Close the database connection */
    mysql_close(conn);
}

char* get_date_time() {
    time_t rawtime;
    struct tm* info;
    char date_time[80];

    time(&rawtime);
    info = localtime(&rawtime);
    strftime(date_time, 80, "%Y-%m-%d %H:%M:%S", info);

    return strdup(date_time);
}

void print_catalog() {
    /* Open the database connection */
    MYSQL* conn;
    conn = mysql_init(NULL);
    if (mysql_real_connect(conn, "localhost", "web_user", "web_pass", "lorax", 0, NULL, 0) == NULL) {
        print_sql_error(conn);
    }

    /* Query the database for all nodes and their serial numbers  */
    if (mysql_query(conn, "SELECT node_id, serial_number, last_transmission, gps_latitude, gps_longitude FROM Node")) {
        print_sql_error(conn);
    }

    /* Store the query result and count number of fields and rows */
    MYSQL_RES* result = mysql_store_result(conn);
    int num_fields = mysql_num_fields(result);
    int num_rows = mysql_num_rows(result);

    /* Check to see if the result set is empty */
    if (result == NULL) {
        print_sql_error(conn);
    }

    printf("\nFeather M0 Node Catalog : %s\n", get_date_time());
    printf("-------------------------------------------------------------------------------------------------------------\n");
    printf("node\tserial_number\t\t\t\t\tlast_transmission\tgps_latitude\tgps_longitude\n");
    printf("-------------------------------------------------------------------------------------------------------------\n");

    /* Print all field values for each row in the result set */
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        for (int i = 0; i < num_fields; i++) {
            printf("%s\t", row[i] ? row[i] : "NULL");
        }
        printf("\n");
    }
    printf("-------------------------------------------------------------------------------------------------------------\n\n");
}

void print_sql_error(MYSQL* conn) {
    fprintf(stderr, "%s\n", mysql_error(conn));
    mysql_close(conn);
}