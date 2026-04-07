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
 *    Data stored in "customers.dat" (binary).
 *    Auto-loaded at startup.
 *    Auto-saved after Add / Calculate / Delete.
 * ================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ── Constants ── */
#define MAX_METERS      5
#define MAX_CUSTOMERS   100
#define DATA_FILE       "customers.dat"
#define SERVICE_CHARGE  50.0f

/* Tiered slab limits */
#define SLAB1_LIMIT     100.0f
#define SLAB2_LIMIT     300.0f
#define RATE_SLAB1      4.00f
#define RATE_SLAB2      6.00f
#define RATE_SLAB3      9.50f

/* ── Structures ── */
typedef struct {
    int   meter_id;
    float prev_reading;
    float curr_reading;
    float units;
    float bill;
} Meter;

typedef struct {
    int    customer_id;
    char   name[50];
    char   address[100];
    int    meter_count;
    Meter  meters[MAX_METERS];
    float  last_total_bill;
} Customer;

/* ── Globals ── */
Customer customers[MAX_CUSTOMERS];
int      count = 0;

/* ════════════════════════════════════════════════
   TIERED BILL CALCULATOR
   Returns energy charge for `units` consumed.
   ════════════════════════════════════════════════ */
float tieredCharge(float units) {
    if (units <= 0)              return 0.0f;
    if (units <= SLAB1_LIMIT)
        return units * RATE_SLAB1;
    if (units <= SLAB2_LIMIT)
        return (SLAB1_LIMIT              * RATE_SLAB1)
             + ((units - SLAB1_LIMIT)   * RATE_SLAB2);
    return (SLAB1_LIMIT                * RATE_SLAB1)
         + ((SLAB2_LIMIT - SLAB1_LIMIT)* RATE_SLAB2)
         + ((units - SLAB2_LIMIT)      * RATE_SLAB3);
}

/* ════════════════════════════════════════════════
   FILE I/O
   ════════════════════════════════════════════════ */
void saveToFile() {
    FILE *fp = fopen(DATA_FILE, "wb");
    if (!fp) {
        printf("[ERROR] Cannot open '%s' for writing!\n", DATA_FILE);
        return;
    }
    fwrite(&count,    sizeof(int),      1,     fp);
    fwrite(customers, sizeof(Customer), count, fp);
    fclose(fp);
    printf("[INFO] Data saved to '%s' (%d record(s)).\n", DATA_FILE, count);
}

void loadFromFile() {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        printf("[INFO] No saved data found. Starting fresh.\n");
        return;
    }
    int loaded = 0;
    if (fread(&loaded, sizeof(int), 1, fp) == 1
        && loaded >= 0 && loaded <= MAX_CUSTOMERS
        && (int)fread(customers, sizeof(Customer), loaded, fp) == loaded) {
        count = loaded;
        printf("[INFO] Loaded %d customer(s) from '%s'.\n", count, DATA_FILE);
    } else {
        printf("[WARNING] Data file corrupted. Starting fresh.\n");
        count = 0;
    }
    fclose(fp);
}

/* ════════════════════════════════════════════════
   HELPERS
   ════════════════════════════════════════════════ */
int findCustomer(int id) {
    for (int i = 0; i < count; i++)
        if (customers[i].customer_id == id) return i;
    return -1;
}

void clearBuf() { int c; while ((c = getchar()) != '\n' && c != EOF); }

void printSlab(const char *label, float units, float rate) {
    printf("      %-24s %7.2f units x BDT %4.2f = BDT %8.2f\n",
           label, units, rate, units * rate);
}

/* ════════════════════════════════════════════════
   1. ADD CUSTOMER
   ════════════════════════════════════════════════ */
void addCustomer() {
    if (count >= MAX_CUSTOMERS) {
        printf("\n[ERROR] Customer limit (%d) reached!\n", MAX_CUSTOMERS);
        return;
    }
    printf("\n--- Add New Customer ---\n");

    int id;
    printf("Enter Customer ID : ");
    if (scanf("%d", &id) != 1) { clearBuf(); printf("[ERROR] Invalid ID!\n"); return; }
    if (findCustomer(id) != -1) {
        printf("[ERROR] ID %d already exists!\n", id);
        clearBuf(); return;
    }

    customers[count].customer_id    = id;
    customers[count].last_total_bill = 0.0f;
    clearBuf();

    printf("Enter Name        : ");
    fgets(customers[count].name, 50, stdin);
    customers[count].name[strcspn(customers[count].name, "\n")] = '\0';

    printf("Enter Address     : ");
    fgets(customers[count].address, 100, stdin);
    customers[count].address[strcspn(customers[count].address, "\n")] = '\0';

    // int mc;
    // printf("Number of Meters (1-%d): ", MAX_METERS);
    // if (scanf("%d", &mc) != 1 || mc < 1 || mc > MAX_METERS) {
    //     printf("[ERROR] Must be 1 to %d.\n", MAX_METERS);
    //     clearBuf(); return;
    // }
    // customers[count].meter_count = mc;

    // for (int i = 0; i < mc; i++) {
    //     printf("\n  -- Meter %d --\n", i + 1);
    //     printf("    Meter ID          : "); scanf("%d", &customers[count].meters[i].meter_id);
    //     printf("    Previous Reading  : "); scanf("%f", &customers[count].meters[i].prev_reading);
    //     printf("    Current  Reading  : "); scanf("%f", &customers[count].meters[i].curr_reading);

    //     float u = customers[count].meters[i].curr_reading
    //             - customers[count].meters[i].prev_reading;
    //     if (u < 0) { printf("    [WARNING] Current < Previous. Units = 0.\n"); u = 0; }
    //     customers[count].meters[i].units = u;
    //     customers[count].meters[i].bill  = 0.0f;
    // }

    int meterIndex = 0;
char choice;

do {
    if (meterIndex >= MAX_METERS) {
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

    float u = customers[count].meters[meterIndex].curr_reading
            - customers[count].meters[meterIndex].prev_reading;

    if (u < 0) {
        printf("    [WARNING] Current < Previous. Units = 0.\n");
        u = 0;
    }

    customers[count].meters[meterIndex].units = u;
    customers[count].meters[meterIndex].bill  = 0.0f;

    meterIndex++;

    // ask user
    printf("\nDo you want to add another meter? (y/n): ");
    scanf(" %c", &choice);   // space before %c important!

} while (choice == 'y' || choice == 'Y');

// finally set meter count
customers[count].meter_count = meterIndex;
    count++;
    printf("\n[OK] Customer added successfully!\n");
    saveToFile();
}

/* ════════════════════════════════════════════════
   2. VIEW ALL CUSTOMERS
   ════════════════════════════════════════════════ */
void viewAllCustomers() {
    if (count == 0) { printf("\nNo customers found!\n"); return; }

    for (int i = 0; i < count; i++) {
        printf("\n+----------------------------------------------+\n");
        printf("  Customer #%-3d  ID: %d\n", i + 1, customers[i].customer_id);
        printf("  Name    : %s\n", customers[i].name);
        printf("  Address : %s\n", customers[i].address);
        printf("  Meters  : %d\n", customers[i].meter_count);

        for (int j = 0; j < customers[i].meter_count; j++) {
            Meter *m = &customers[i].meters[j];
            printf("  |- Meter %d (ID:%d)  Prev:%.2f  Curr:%.2f  Units:%.2f  Bill:BDT %.2f\n",
                   j+1, m->meter_id, m->prev_reading, m->curr_reading, m->units, m->bill);
        }
        if (customers[i].last_total_bill > 0)
            printf("  Last Total Bill : BDT %.2f\n", customers[i].last_total_bill);
        printf("+----------------------------------------------+\n");
    }
}

/* ════════════════════════════════════════════════
   3. CALCULATE BILL  (tiered + service charge)
   ════════════════════════════════════════════════ */
void calculateBill() {
    printf("\nEnter Customer ID to calculate bill: ");
    int id;
    if (scanf("%d", &id) != 1) { clearBuf(); printf("[ERROR] Invalid input!\n"); return; }

    int idx = findCustomer(id);
    if (idx == -1) { printf("[ERROR] Customer ID %d not found!\n", id); return; }

    Customer *c = &customers[idx];
    float grandTotal = 0.0f;

    printf("\n+================================================+\n");
    printf("  ELECTRICITY BILL\n");
    printf("  Customer : %s  (ID: %d)\n", c->name, c->customer_id);
    printf("  Address  : %s\n", c->address);
    printf("+================================================+\n");

    for (int j = 0; j < c->meter_count; j++) {
        Meter *m = &c->meters[j];

        float units = m->curr_reading - m->prev_reading;
        if (units < 0) {
            printf("\n[WARNING] Meter %d: Current < Previous — skipped.\n", j+1);
            m->units = 0; m->bill = 0; continue;
        }
        m->units = units;

        printf("\n  Meter %d  (ID: %d)\n", j+1, m->meter_id);
        printf("  Previous: %.2f  |  Current: %.2f  |  Units: %.2f\n",
               m->prev_reading, m->curr_reading, units);
        printf("  Slab Breakdown:\n");

        float charge = 0.0f;

        if (units == 0) {
            printf("      (no consumption)\n");

        } else if (units <= SLAB1_LIMIT) {
            printSlab("Slab 1 (0-100 units):",   units, RATE_SLAB1);
            charge = units * RATE_SLAB1;

        } else if (units <= SLAB2_LIMIT) {
            printSlab("Slab 1 (0-100 units):",       SLAB1_LIMIT,          RATE_SLAB1);
            printSlab("Slab 2 (101-300 units):", units - SLAB1_LIMIT,      RATE_SLAB2);
            charge = tieredCharge(units);

        } else {
            printSlab("Slab 1 (0-100 units):",       SLAB1_LIMIT,                      RATE_SLAB1);
            printSlab("Slab 2 (101-300 units):",     SLAB2_LIMIT - SLAB1_LIMIT,        RATE_SLAB2);
            printSlab("Slab 3 (301+ units):",        units - SLAB2_LIMIT,              RATE_SLAB3);
            charge = tieredCharge(units);
        }

        m->bill    = charge;
        grandTotal += charge;
        printf("  ------------------------------------------------\n");
        printf("  Meter Subtotal                  : BDT %8.2f\n", m->bill);
    }

    grandTotal         += SERVICE_CHARGE;
    c->last_total_bill  = grandTotal;

    printf("\n+================================================+\n");
    printf("  Rate Guide: Slab1=BDT 4.00 | Slab2=BDT 6.00 | Slab3=BDT 9.50\n");
    printf("  Service Charge                  : BDT %8.2f\n", SERVICE_CHARGE);
    printf("  ================================================\n");
    printf("  TOTAL BILL PAYABLE              : BDT %8.2f\n", grandTotal);
    printf("+================================================+\n");

    saveToFile();
}

/* ════════════════════════════════════════════════
   4. SEARCH CUSTOMER
   ════════════════════════════════════════════════ */
void searchCustomer() {
    printf("\nEnter Customer ID to search: ");
    int id;
    if (scanf("%d", &id) != 1) { clearBuf(); return; }

    int idx = findCustomer(id);
    if (idx == -1) { printf("[ERROR] Customer ID %d not found.\n", id); return; }

    Customer *c = &customers[idx];
    printf("\n  ID      : %d\n",  c->customer_id);
    printf("  Name    : %s\n",  c->name);
    printf("  Address : %s\n",  c->address);
    for (int j = 0; j < c->meter_count; j++) {
        Meter *m = &c->meters[j];
        printf("  Meter %d : Units=%.2f  Bill=BDT %.2f\n", j+1, m->units, m->bill);
    }
    if (c->last_total_bill > 0)
        printf("  Last Total Bill : BDT %.2f\n", c->last_total_bill);
}

/* ════════════════════════════════════════════════
   5. DELETE CUSTOMER
   ════════════════════════════════════════════════ */
void deleteCustomer() {
    printf("\nEnter Customer ID to delete: ");
    int id;
    if (scanf("%d", &id) != 1) { clearBuf(); return; }

    int idx = findCustomer(id);
    if (idx == -1) { printf("[ERROR] Customer ID %d not found.\n", id); return; }

    for (int i = idx; i < count - 1; i++) customers[i] = customers[i + 1];
    count--;
    printf("[OK] Customer ID %d deleted.\n", id);
    saveToFile();
}

/* ════════════════════════════════════════════════
   MAIN MENU
   ════════════════════════════════════════════════ */
int main() {
    printf("\n==============================================\n");
    printf("   ELECTRICITY BILL MANAGEMENT SYSTEM\n");
    printf("==============================================\n");
    loadFromFile();

    int choice;
    while (1) {
        printf("\n====== MAIN MENU ======\n");
        printf("1. Add Customer\n");
        printf("2. View All Customers\n");
        printf("3. Calculate Bill\n");
        printf("4. Search Customer\n");
        printf("5. Delete Customer\n");
        // printf("6. Save Data Manually\n");
        printf("6. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            clearBuf();
            printf("[ERROR] Invalid input!\n");
            continue;
        }

        switch (choice) {
            case 1: addCustomer();      break;
            case 2: viewAllCustomers(); break;
            case 3: calculateBill();    break;
            case 4: searchCustomer();   break;
            case 5: deleteCustomer();   break;
            case 6: saveToFile();       break;
            case 7:
                saveToFile();
                printf("Goodbye!\n");
                exit(0);
            default:
                printf("[ERROR] Invalid choice! Enter 1-7.\n");
        }
    }
    return 0;
}