/*
 * ================================================
 *    ELECTRICITY BILL MANAGEMENT SYSTEM
 *    Version: 100%
 *    Features: Add / View / Calculate / Search / Delete / Save / Load / Exit
 * ================================================
 *
 *  TIERED RATE STRUCTURE (per unit):
 *    Slab 1 :   0 – 100  units  →  BDT  4.00
 *    Slab 2 : 101 – 300  units  →  BDT  6.00
 *    Slab 3 : 300+       units  →  BDT  9.50
 *  SERVICE CHARGE : BDT 50.00 (flat, per customer)
 *
 *  FILE PERSISTENCE:
 *    Data stored in "customers.txt" (human-readable text).
 *    Auto-loaded at startup.
 *    Auto-saved after Add / Calculate / Delete.
 *
 *  EXPORT:
 *    All customers  → "all_customers_report.txt"
 *    Single customer by ID → "customer_<ID>.txt"
 * ================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── Constants ── */
#define MAX_METERS 5
#define MAX_CUSTOMERS 100
#define DATA_FILE "customers.txt"
#define SERVICE_CHARGE 50.0f

/* Tiered slab limits */
#define SLAB1_LIMIT 100.0f
#define SLAB2_LIMIT 300.0f
#define RATE_SLAB1 4.00f
#define RATE_SLAB2 6.00f
#define RATE_SLAB3 9.50f

/* ── Structures ── */
typedef struct
{
    int meter_id;
    float prev_reading;
    float curr_reading;
    float units;
    float bill;
} Meter;

typedef struct
{
    int customer_id;
    char name[50];
    char address[100];
    int meter_count;
    Meter meters[MAX_METERS];
    float last_total_bill;
} Customer;

/* ── Globals ── */
Customer customers[MAX_CUSTOMERS];
int count = 0;

/* ════════════════════════════════════════════════
   TIERED BILL CALCULATOR
   ════════════════════════════════════════════════ */
float tieredCharge(float units)
{
    if (units <= 0)
        return 0.0f;
    if (units <= SLAB1_LIMIT)
        return units * RATE_SLAB1;
    if (units <= SLAB2_LIMIT)
        return (SLAB1_LIMIT * RATE_SLAB1) + ((units - SLAB1_LIMIT) * RATE_SLAB2);
    return (SLAB1_LIMIT * RATE_SLAB1) + ((SLAB2_LIMIT - SLAB1_LIMIT) * RATE_SLAB2) + ((units - SLAB2_LIMIT) * RATE_SLAB3);
}

/* ════════════════════════════════════════════════
   FILE I/O  —  Plain-text (.txt) format
   ════════════════════════════════════════════════ */

/* ── SAVE: writes all customer records in readable key=value lines ── */
void saveToFile()
{
    FILE *fp = fopen(DATA_FILE, "w");
    if (!fp)
    {
        printf("[ERROR] Cannot open '%s' for writing!\n", DATA_FILE);
        return;
    }

    fprintf(fp, "# Electricity Bill Management System - Customer Database\n");
    fprintf(fp, "# DO NOT edit manually unless you know the format.\n");
    fprintf(fp, "TOTAL_CUSTOMERS=%d\n\n", count);

    for (int i = 0; i < count; i++)
    {
        Customer *c = &customers[i];
        fprintf(fp, "[CUSTOMER]\n");
        fprintf(fp, "customer_id=%d\n",    c->customer_id);
        fprintf(fp, "name=%s\n",           c->name);
        fprintf(fp, "address=%s\n",        c->address);
        fprintf(fp, "meter_count=%d\n",    c->meter_count);
        fprintf(fp, "last_total_bill=%.2f\n", c->last_total_bill);

        for (int j = 0; j < c->meter_count; j++)
        {
            Meter *m = &c->meters[j];
            fprintf(fp, "  [METER_%d]\n",         j + 1);
            fprintf(fp, "  meter_id=%d\n",         m->meter_id);
            fprintf(fp, "  prev_reading=%.2f\n",   m->prev_reading);
            fprintf(fp, "  curr_reading=%.2f\n",   m->curr_reading);
            fprintf(fp, "  units=%.2f\n",          m->units);
            fprintf(fp, "  bill=%.2f\n",           m->bill);
            fprintf(fp, "  [/METER_%d]\n",         j + 1);
        }

        fprintf(fp, "[/CUSTOMER]\n\n");
    }

    fclose(fp);
    printf("[INFO] Data saved to '%s' (%d record(s)).\n", DATA_FILE, count);
}

/* ── LOAD: reads the plain-text customer database ── */
void loadFromFile()
{
    FILE *fp = fopen(DATA_FILE, "r");
    if (!fp)
    {
        printf("[INFO] No saved data found ('%s'). Starting fresh.\n", DATA_FILE);
        return;
    }

    char line[256];
    int ci = -1;      /* current customer index  */
    int mi = -1;      /* current meter index     */
    count  =  0;

    while (fgets(line, sizeof(line), fp))
    {
        /* strip trailing newline */
        line[strcspn(line, "\r\n")] = '\0';

        /* skip blank lines and comments */
        if (line[0] == '\0' || line[0] == '#')
            continue;

        /* --- section markers --- */
        if (strcmp(line, "[CUSTOMER]") == 0)
        {
            ci = count;
            mi = -1;
            memset(&customers[ci], 0, sizeof(Customer));
            continue;
        }
        if (strcmp(line, "[/CUSTOMER]") == 0)
        {
            if (ci >= 0)
                count++;
            ci = -1;
            continue;
        }
        if (ci >= 0 && strncmp(line, "  [METER_", 9) == 0 && line[strlen(line)-1] != '/')
        {
            /* opening meter tag */
            mi++;
            continue;
        }
        if (ci >= 0 && strncmp(line, "  [/METER_", 10) == 0)
        {
            /* closing meter tag — mi stays so we can count */
            continue;
        }

        /* --- key=value pairs --- */
        char key[64], val[200];
        if (sscanf(line, " %63[^=]=%199[^\n]", key, val) != 2)
            continue;

        /* top-level header */
        if (strcmp(key, "TOTAL_CUSTOMERS") == 0)
            continue; /* we count ourselves */

        if (ci < 0)
            continue;

        Customer *c = &customers[ci];

        if (mi < 0) /* customer-level fields */
        {
            if      (strcmp(key, "customer_id")     == 0) c->customer_id     = atoi(val);
            else if (strcmp(key, "name")             == 0) strncpy(c->name,    val, 49);
            else if (strcmp(key, "address")          == 0) strncpy(c->address, val, 99);
            else if (strcmp(key, "meter_count")      == 0) c->meter_count     = atoi(val);
            else if (strcmp(key, "last_total_bill")  == 0) c->last_total_bill = (float)atof(val);
        }
        else /* meter-level fields */
        {
            Meter *m = &c->meters[mi];
            if      (strcmp(key, "meter_id")      == 0) m->meter_id      = atoi(val);
            else if (strcmp(key, "prev_reading")   == 0) m->prev_reading  = (float)atof(val);
            else if (strcmp(key, "curr_reading")   == 0) m->curr_reading  = (float)atof(val);
            else if (strcmp(key, "units")          == 0) m->units         = (float)atof(val);
            else if (strcmp(key, "bill")           == 0) m->bill          = (float)atof(val);
        }
    }

    fclose(fp);

    if (count > 0)
        printf("[INFO] Loaded %d customer(s) from '%s'.\n", count, DATA_FILE);
    else
        printf("[INFO] Database file found but no valid records. Starting fresh.\n");
}

/* ════════════════════════════════════════════════
   EXPORT HELPERS
   ════════════════════════════════════════════════ */

/* Writes one customer block to an open FILE* in human-readable English */
void writeCustomerReport(FILE *fp, Customer *c)
{
    fprintf(fp, "============================================================\n");
    fprintf(fp, "  Customer ID  : %d\n",   c->customer_id);
    fprintf(fp, "  Name         : %s\n",   c->name);
    fprintf(fp, "  Address      : %s\n",   c->address);
    fprintf(fp, "  Meters       : %d\n",   c->meter_count);
    fprintf(fp, "------------------------------------------------------------\n");

    for (int j = 0; j < c->meter_count; j++)
    {
        Meter *m = &c->meters[j];
        fprintf(fp, "  Meter %d  (Meter ID: %d)\n", j + 1, m->meter_id);
        fprintf(fp, "    Previous Reading  : %.2f units\n",  m->prev_reading);
        fprintf(fp, "    Current  Reading  : %.2f units\n",  m->curr_reading);
        fprintf(fp, "    Units Consumed    : %.2f units\n",  m->units);
        fprintf(fp, "    Meter Charge      : BDT %.2f\n",    m->bill);
        fprintf(fp, "\n");
    }

    fprintf(fp, "  Service Charge    : BDT %.2f\n",  SERVICE_CHARGE);
    fprintf(fp, "  TOTAL BILL        : BDT %.2f\n",  c->last_total_bill);
    fprintf(fp, "============================================================\n\n");
}

/* Export ALL customers to a readable report file */
void exportAllCustomersReport()
{
    if (count == 0)
    {
        printf("\n[INFO] No customers to export.\n");
        return;
    }

    const char *filename = "all_customers_report.txt";
    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        printf("[ERROR] Cannot create '%s'!\n", filename);
        return;
    }

    /* timestamp */
    time_t now = time(NULL);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(fp, "ELECTRICITY BILL MANAGEMENT SYSTEM\n");
    fprintf(fp, "All Customers Report\n");
    fprintf(fp, "Generated : %s\n", timebuf);
    fprintf(fp, "Total Customers : %d\n\n", count);

    for (int i = 0; i < count; i++)
        writeCustomerReport(fp, &customers[i]);

    fclose(fp);
    printf("\n[OK] All customers exported to '%s'.\n", filename);
}

/* Export a single customer by ID to customer_<ID>.txt */
void exportSingleCustomer(int id)
{
    int idx = -1;
    for (int i = 0; i < count; i++)
        if (customers[i].customer_id == id)
        { idx = i; break; }

    if (idx == -1)
    {
        printf("[ERROR] Customer ID %d not found.\n", id);
        return;
    }

    char filename[64];
    snprintf(filename, sizeof(filename), "customer_%d.txt", id);

    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        printf("[ERROR] Cannot create '%s'!\n", filename);
        return;
    }

    time_t now = time(NULL);
    char timebuf[64];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", localtime(&now));

    fprintf(fp, "ELECTRICITY BILL MANAGEMENT SYSTEM\n");
    fprintf(fp, "Customer Report\n");
    fprintf(fp, "Generated : %s\n\n", timebuf);

    writeCustomerReport(fp, &customers[idx]);

    fclose(fp);
    printf("[OK] Customer %d's info saved to '%s'.\n", id, filename);
}

/* ════════════════════════════════════════════════
   HELPERS
   ════════════════════════════════════════════════ */
int findCustomer(int id)
{
    for (int i = 0; i < count; i++)
        if (customers[i].customer_id == id)
            return i;
    return -1;
}

void clearBuf()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void printSlab(const char *label, float units, float rate)
{
    printf("      %-24s %7.2f units x BDT %4.2f = BDT %8.2f\n",
           label, units, rate, units * rate);
}

/* ════════════════════════════════════════════════
   1. ADD CUSTOMER
   ════════════════════════════════════════════════ */
void addCustomer()
{
    if (count >= MAX_CUSTOMERS)
    {
        printf("\n[ERROR] Customer limit (%d) reached!\n", MAX_CUSTOMERS);
        return;
    }
    printf("\n--- Add New Customer ---\n");

    int id;
    printf("Enter Customer ID : ");
    if (scanf("%d", &id) != 1)
    {
        clearBuf();
        printf("[ERROR] Invalid ID!\n");
        return;
    }
    if (findCustomer(id) != -1)
    {
        printf("[ERROR] ID %d already exists!\n", id);
        clearBuf();
        return;
    }

    customers[count].customer_id   = id;
    customers[count].last_total_bill = 0.0f;
    clearBuf();

    printf("Enter Name        : ");
    fgets(customers[count].name, 50, stdin);
    customers[count].name[strcspn(customers[count].name, "\n")] = '\0';

    printf("Enter Address     : ");
    fgets(customers[count].address, 100, stdin);
    customers[count].address[strcspn(customers[count].address, "\n")] = '\0';

    int meterIndex = 0;
    char choice;

    do
    {
        if (meterIndex >= MAX_METERS)
        {
            printf("\n[INFO] Maximum %d meters reached!\n", MAX_METERS);
            break;
        }

        printf("\n  -- Meter %d --\n", meterIndex + 1);
        printf("    Meter ID          : ");
        scanf("%d", &customers[count].meters[meterIndex].meter_id);

        printf("    Previous Reading  : ");
        scanf("%f", &customers[count].meters[meterIndex].prev_reading);

        printf("    Current  Reading  : ");
        scanf("%f", &customers[count].meters[meterIndex].curr_reading);

        float u = customers[count].meters[meterIndex].curr_reading -
                  customers[count].meters[meterIndex].prev_reading;
        if (u < 0)
        {
            printf("    [WARNING] Current < Previous. Units set to 0.\n");
            u = 0;
        }
        customers[count].meters[meterIndex].units = u;
        customers[count].meters[meterIndex].bill  = 0.0f;
        meterIndex++;

        printf("\nAdd another meter? (y/n): ");
        scanf(" %c", &choice);

    } while (choice == 'y' || choice == 'Y');

    customers[count].meter_count = meterIndex;
    count++;
    printf("\n[OK] Customer added successfully!\n");
    saveToFile();
}

/* ════════════════════════════════════════════════
   2. VIEW ALL CUSTOMERS  (console display)
   ════════════════════════════════════════════════ */
void viewAllCustomers()
{
    if (count == 0)
    {
        printf("\nNo customers found!\n");
        return;
    }

    for (int i = 0; i < count; i++)
    {
        printf("\n+----------------------------------------------+\n");
        printf("  Customer #%-3d  ID: %d\n", i + 1, customers[i].customer_id);
        printf("  Name    : %s\n", customers[i].name);
        printf("  Address : %s\n", customers[i].address);
        printf("  Meters  : %d\n", customers[i].meter_count);

        for (int j = 0; j < customers[i].meter_count; j++)
        {
            Meter *m = &customers[i].meters[j];
            printf("  |- Meter %d (ID:%d)  Prev:%.2f  Curr:%.2f  Units:%.2f  Bill:BDT %.2f\n",
                   j + 1, m->meter_id, m->prev_reading, m->curr_reading, m->units, m->bill);
        }
        if (customers[i].last_total_bill > 0)
            printf("  Last Total Bill : BDT %.2f\n", customers[i].last_total_bill);
        printf("+----------------------------------------------+\n");
    }

    /* also write a readable report file */
    // exportAllCustomersReport();
}

/* ════════════════════════════════════════════════
   3. CALCULATE BILL  (tiered + service charge)
   ════════════════════════════════════════════════ */
void calculateBill()
{
    printf("\nEnter Customer ID to calculate bill: ");
    int id;
    if (scanf("%d", &id) != 1)
    {
        clearBuf();
        printf("[ERROR] Invalid input!\n");
        return;
    }

    int idx = findCustomer(id);
    if (idx == -1)
    {
        printf("[ERROR] Customer ID %d not found!\n", id);
        return;
    }

    Customer *c = &customers[idx];
    float grandTotal = 0.0f;

    printf("\n+================================================+\n");
    printf("  ELECTRICITY BILL\n");
    printf("  Customer : %s  (ID: %d)\n", c->name, c->customer_id);
    printf("  Address  : %s\n", c->address);
    printf("+================================================+\n");

    for (int j = 0; j < c->meter_count; j++)
    {
        Meter *m = &c->meters[j];
        float units = m->curr_reading - m->prev_reading;

        if (units < 0)
        {
            printf("\n[WARNING] Meter %d: Current < Previous — skipped.\n", j + 1);
            m->units = 0;
            m->bill  = 0;
            continue;
        }
        m->units = units;

        printf("\n  Meter %d  (ID: %d)\n", j + 1, m->meter_id);
        printf("  Previous: %.2f  |  Current: %.2f  |  Units: %.2f\n",
               m->prev_reading, m->curr_reading, units);
        printf("  Slab Breakdown:\n");

        float charge = 0.0f;

        if (units == 0)
        {
            printf("      (no consumption)\n");
        }
        else if (units <= SLAB1_LIMIT)
        {
            printSlab("Slab 1 (0-100 units):", units, RATE_SLAB1);
            charge = units * RATE_SLAB1;
        }
        else if (units <= SLAB2_LIMIT)
        {
            printSlab("Slab 1 (0-100 units):",   SLAB1_LIMIT,        RATE_SLAB1);
            printSlab("Slab 2 (101-300 units):", units - SLAB1_LIMIT, RATE_SLAB2);
            charge = tieredCharge(units);
        }
        else
        {
            printSlab("Slab 1 (0-100 units):",   SLAB1_LIMIT,                  RATE_SLAB1);
            printSlab("Slab 2 (101-300 units):", SLAB2_LIMIT - SLAB1_LIMIT,    RATE_SLAB2);
            printSlab("Slab 3 (301+ units):",    units - SLAB2_LIMIT,           RATE_SLAB3);
            charge = tieredCharge(units);
        }

        m->bill   = charge;
        grandTotal += charge;
        printf("  ------------------------------------------------\n");
        printf("  Meter Subtotal                  : BDT %8.2f\n", m->bill);
    }

    grandTotal      += SERVICE_CHARGE;
    c->last_total_bill = grandTotal;

    printf("\n+================================================+\n");
    printf("  Rate Guide: Slab1=BDT 4.00 | Slab2=BDT 6.00 | Slab3=BDT 9.50\n");
    printf("  Service Charge                  : BDT %8.2f\n", SERVICE_CHARGE);
    printf("  ================================================\n");
    printf("  TOTAL BILL PAYABLE              : BDT %8.2f\n", grandTotal);
    printf("+================================================+\n");

    saveToFile();
}

/* ════════════════════════════════════════════════
   4. SEARCH CUSTOMER  (with optional export)
   ════════════════════════════════════════════════ */
void searchCustomer()
{
    printf("\nEnter Customer ID to search: ");
    int id;
    if (scanf("%d", &id) != 1)
    {
        clearBuf();
        return;
    }

    int idx = findCustomer(id);
    if (idx == -1)
    {
        printf("[ERROR] Customer ID %d not found.\n", id);
        return;
    }

    Customer *c = &customers[idx];
    printf("\n  ID      : %d\n", c->customer_id);
    printf("  Name    : %s\n",   c->name);
    printf("  Address : %s\n",   c->address);

    for (int j = 0; j < c->meter_count; j++)
    {
        Meter *m = &c->meters[j];
        printf("  Meter %d (ID:%d) : Units=%.2f  Bill=BDT %.2f\n",
               j + 1, m->meter_id, m->units, m->bill);
    }
    if (c->last_total_bill > 0)
        printf("  Last Total Bill : BDT %.2f\n", c->last_total_bill);

    /* Ask if user wants to download/export this customer's info */
    char choice;
    printf("\nDo you want to save this customer's info to a file? (y/n): ");
    scanf(" %c", &choice);
    if (choice == 'y' || choice == 'Y')
        exportSingleCustomer(id);
}

/* ════════════════════════════════════════════════
   5. DELETE CUSTOMER
   ════════════════════════════════════════════════ */
void deleteCustomer()
{
    printf("\nEnter Customer ID to delete: ");
    int id;
    if (scanf("%d", &id) != 1)
    {
        clearBuf();
        return;
    }

    int idx = findCustomer(id);
    if (idx == -1)
    {
        printf("[ERROR] Customer ID %d not found.\n", id);
        return;
    }

    for (int i = idx; i < count - 1; i++)
        customers[i] = customers[i + 1];
    count--;
    printf("[OK] Customer ID %d deleted.\n", id);
    saveToFile();
}

/* ════════════════════════════════════════════════
   MAIN MENU
   ════════════════════════════════════════════════ */
int main()
{
    printf("\n==============================================\n");
    printf("   ELECTRICITY BILL MANAGEMENT SYSTEM\n");
    printf("==============================================\n");
    loadFromFile();

    int choice;
    while (1)
    {
        printf("\n====== MAIN MENU ======\n");
        printf("1. Add Customer\n");
        printf("2. View All Customers\n");
        printf("3. Calculate Bill\n");
        printf("4. Search Customer\n");
        printf("5. Delete Customer\n");
        printf("6. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1)
        {
            clearBuf();
            printf("[ERROR] Invalid input!\n");
            continue;
        }

        switch (choice)
        {
        case 1: addCustomer();     break;
        case 2: viewAllCustomers(); break;
        case 3: calculateBill();   break;
        case 4: searchCustomer();  break;
        case 5: deleteCustomer();  break;
        case 6:
            saveToFile();
            printf("Goodbye!\n");
            exit(0);
        default:
            printf("[ERROR] Invalid choice! Enter 1-6.\n");
        }
    }
    return 0;
}