// Define macros to exclude conflicting Windows API functions
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOKEYSTATES
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT          // Exclude DrawText function
#define NOGDI               // Exclude GDI functions
#define NOKERNEL
#define NOUSER              // Exclude User functions like CloseWindow, ShowCursor
#define NONLS
#define NOMB
#define NOMEMMGR
#define NOMETAFILE
#define NOMINMAX            // Exclude min/max macros
#define NOMSG
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND             // Exclude PlaySound function
#define NOTEXTMETRIC
#define NOWH
#define NOWINOFFSETS
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX

// Include Windows headers
#include <windows.h>

// Include raylib
#include "raylib.h"

// Other includes
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>             // For random number generation
#include <math.h>             // For ceil function
#include "include/curl/curl.h" // cURL for downloading JSON data
#include <limits.h>
// Memory struct for cURL response
struct Memory {
    char *response;
    size_t size;
};

// Callback function for cURL to write data into memory
static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct Memory *mem = (struct Memory *)userp;

    char *ptr = realloc(mem->response, mem->size + realsize + 1);
    if (!ptr) {
        fprintf(stderr, "Not enough memory!\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->response[mem->size] = 0;

    return realsize;
}

// Download JSON file using cURL
int download_json(const char *url, const char *output_file) {
    CURL *curl;
    CURLcode res;
    struct Memory chunk = {0};

    curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Curl initialization failed!\n");
        return 1;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // For testing purposes
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L); // For testing purposes
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    // Perform the request
    res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        free(chunk.response);
        curl_easy_cleanup(curl);
        return 1;
    }

    // Write the response to a file
    FILE *file = fopen(output_file, "w");
    if (!file) {
        fprintf(stderr, "Could not open file for writing: %s\n", output_file);
        free(chunk.response);
        curl_easy_cleanup(curl);
        return 1;
    }
    fwrite(chunk.response, 1, chunk.size, file);
    fclose(file);

    free(chunk.response);
    curl_easy_cleanup(curl);

    return 0;
}

// Function to calculate total attack power with critical hit chance
typedef struct {
    int attackCount;  // Total attacks performed by the unit
    int critThreshold;  // Number of attacks needed for a critical hit
} AttackData;

// Initialize attack data for each unit type based on critChance
AttackData initializeAttackData(int critChance) {
    AttackData data;
    data.attackCount = 0;
    if (critChance > 0) {
        data.critThreshold = (int)(1.0 / (critChance / 100.0));
    } else {
        data.critThreshold = INT_MAX; // Set to a high value if critChance is 0
    }
    return data;
}

// Function to calculate total attack power with scheduled critical hits
long long int calculateAttackPower(int unitAttackPower, long long int unitCount, AttackData *attackData, FILE *logFile, const char *unitName, int round) {
    long long int baseAttack = (long long int)unitAttackPower * unitCount;
    attackData->attackCount++;

    // Apply critical hit every critThreshold attacks
    if (attackData->attackCount >= attackData->critThreshold) {
        baseAttack = (long long int)(baseAttack * 1.5); // Increase attack power by 50%
        attackData->attackCount = 0; // Reset attack count after critical hit
        fprintf(logFile, "Round %d: %s lands a SCHEDULED CRITICAL HIT! Attack power increased by 50%% to %lld.\n", round, unitName, baseAttack);
    }

    return baseAttack;
}
// Function to calculate total defense power
long long int calculateDefensePower(int unitDefensePower, long long int unitCount) {
    return (long long int)unitDefensePower * unitCount;
}

// Function to calculate net damage
long long int calculateNetDamage(long long int attackPower, long long int defensePower) {
    if (attackPower == 0) {
        return 0;
    }
    long long int netDamage = attackPower - defensePower;
    if (netDamage <= 0) {
        // Calculate minimal damage based on a percentage of attack power
        netDamage = attackPower * 5 / 100; // 5% of attack power as minimum damage
        if (netDamage == 0) {
            netDamage = 1; // Ensure at least 1 point of damage
        }
    }
    return netDamage;
}

// Function to calculate units lost
long long int calculateUnitsLost(long long int damage, long long int unitHealth) {
    if (unitHealth == 0) {
        return 0;
    }
    long long int unitsLost = damage / unitHealth;
    if (unitsLost == 0 && damage > 0) {
        unitsLost = 1; // Ensure at least one unit is lost if there's any damage
    }
    return unitsLost > 0 ? unitsLost : 0;
}

// Function to apply fatigue effect to attack and defense power
void applyFatigueEffect(int *attackPower, int *defensePower, float fatiguePercentage) {
    *attackPower = *attackPower * (100.0f - fatiguePercentage) / 100.0f;
    if (*attackPower < 1) *attackPower = 1; // Minimum attack power is 1

    *defensePower = *defensePower * (100.0f - fatiguePercentage) / 100.0f;
    if (*defensePower < 1) *defensePower = 1; // Minimum defense power is 1
}
// Function to read JSON data from a file
char* readJsonFromFile(const char* filePath) {
    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        fprintf(stderr, "Cannot open file: %s\n", filePath);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(file);
        return NULL;
    }

    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0'; // Null-terminate

    fclose(file);
    return buffer;
}

// Helper function to extract integer value from JSON
int extractIntValue(const char* source, const char* key) {
    char* pos = strstr(source, key);
    if (pos) {
        pos = strchr(pos, ':');
        if (pos) {
            pos++;
            while (*pos == ' ' || *pos == '"' || *pos == '\'') pos++;
            return atoi(pos);
        }
    }
    fprintf(stderr, "Error: Key %s not found in JSON data.\n", key);
    return 0; // Default value if key not found
}

// Helper function to extract long long integer value from JSON
long long int extractLongLongIntValue(const char* source, const char* key) {
    char* pos = strstr(source, key);
    if (pos) {
        pos = strchr(pos, ':');
        if (pos) {
            pos++;
            while (*pos == ' ' || *pos == '"' || *pos == '\'') pos++;
            return atoll(pos);
        }
    }
    return 0;
}

// Helper function to extract string value from JSON
void extractStringValue(const char* source, const char* key, char* dest, size_t destSize) {
    char* pos = strstr(source, key);
    if (pos) {
        pos = strchr(pos, ':');
        if (pos) {
            pos++;
            while (*pos == ' ' || *pos == '"' || *pos == '\'') pos++;
            char* end = strchr(pos, '"');
            if (end) {
                size_t len = end - pos;
                if (len < destSize) {
                    strncpy(dest, pos, len);
                    dest[len] = '\0';
                } else {
                    strncpy(dest, pos, destSize - 1);
                    dest[destSize - 1] = '\0';
                }
            }
        }
    }
}

// Function to parse unit attributes from JSON data manually
void jsonVerisiniIsleVeBirimOzellikleriniAyarla(const char* jsonVerisi,
    int *piyadeSaldiri, int *piyadeSavunma, int *piyadeSaglik, int *piyadeKritikSans,
    int *okcuSaldiri, int *okcuSavunma, int *okcuSaglik, int *okcuKritikSans,
    int *suvariSaldiri, int *suvariSavunma, int *suvariSaglik, int *suvariKritikSans,
    int *kusatmaSaldiri, int *kusatmaSavunma, int *kusatmaSaglik, int *kusatmaKritikSans,
    int *orkSaldiri, int *orkSavunma, int *orkSaglik, int *orkKritikSans,
    int *mizrakciSaldiri, int *mizrakciSavunma, int *mizrakciSaglik, int *mizrakciKritikSans,
    int *vargSaldiri, int *vargSavunma, int *vargSaglik, int *vargKritikSans,
    int *trolSaldiri, int *trolSavunma, int *trolSaglik, int *trolKritikSans) {

    // Piyadeler
    char* piyadeStr = strstr(jsonVerisi, "\"piyadeler\":");
    if (piyadeStr != NULL) {
        *piyadeSaldiri = extractIntValue(piyadeStr, "\"saldiri\"");
        *piyadeSavunma = extractIntValue(piyadeStr, "\"savunma\"");
        *piyadeSaglik = extractIntValue(piyadeStr, "\"saglik\"");
        *piyadeKritikSans = extractIntValue(piyadeStr, "\"kritik_sans\"");
    }

    // Okçular
    char* okcuStr = strstr(jsonVerisi, "\"okcular\":");
    if (okcuStr != NULL) {
        *okcuSaldiri = extractIntValue(okcuStr, "\"saldiri\"");
        *okcuSavunma = extractIntValue(okcuStr, "\"savunma\"");
        *okcuSaglik = extractIntValue(okcuStr, "\"saglik\"");
        *okcuKritikSans = extractIntValue(okcuStr, "\"kritik_sans\"");
    }

    // Süvariler
    char* suvariStr = strstr(jsonVerisi, "\"suvariler\":");
    if (suvariStr != NULL) {
        *suvariSaldiri = extractIntValue(suvariStr, "\"saldiri\"");
        *suvariSavunma = extractIntValue(suvariStr, "\"savunma\"");
        *suvariSaglik = extractIntValue(suvariStr, "\"saglik\"");
        *suvariKritikSans = extractIntValue(suvariStr, "\"kritik_sans\"");
    }

    // Kuþatma Makineleri
    char* kusatmaStr = strstr(jsonVerisi, "\"kusatma_makineleri\":");
    if (kusatmaStr != NULL) {
        *kusatmaSaldiri = extractIntValue(kusatmaStr, "\"saldiri\"");
        *kusatmaSavunma = extractIntValue(kusatmaStr, "\"savunma\"");
        *kusatmaSaglik = extractIntValue(kusatmaStr, "\"saglik\"");
        *kusatmaKritikSans = extractIntValue(kusatmaStr, "\"kritik_sans\"");
    }

    // Ork Dövüþçüleri
    char* orkStr = strstr(jsonVerisi, "\"ork_dovusculeri\":");
    if (orkStr != NULL) {
        *orkSaldiri = extractIntValue(orkStr, "\"saldiri\"");
        *orkSavunma = extractIntValue(orkStr, "\"savunma\"");
        *orkSaglik = extractIntValue(orkStr, "\"saglik\"");
        *orkKritikSans = extractIntValue(orkStr, "\"kritik_sans\"");
    }

    // Mýzrakçýlar
    char* mizrakciStr = strstr(jsonVerisi, "\"mizrakcilar\":");
    if (mizrakciStr != NULL) {
        *mizrakciSaldiri = extractIntValue(mizrakciStr, "\"saldiri\"");
        *mizrakciSavunma = extractIntValue(mizrakciStr, "\"savunma\"");
        *mizrakciSaglik = extractIntValue(mizrakciStr, "\"saglik\"");
        *mizrakciKritikSans = extractIntValue(mizrakciStr, "\"kritik_sans\"");
    }

    // Varg Binicileri
    char* vargStr = strstr(jsonVerisi, "\"varg_binicileri\":");
    if (vargStr != NULL) {
        *vargSaldiri = extractIntValue(vargStr, "\"saldiri\"");
        *vargSavunma = extractIntValue(vargStr, "\"savunma\"");
        *vargSaglik = extractIntValue(vargStr, "\"saglik\"");
        *vargKritikSans = extractIntValue(vargStr, "\"kritik_sans\"");
    }

    // Troller
    char* trolStr = strstr(jsonVerisi, "\"troller\":");
    if (trolStr != NULL) {
        *trolSaldiri = extractIntValue(trolStr, "\"saldiri\"");
        *trolSavunma = extractIntValue(trolStr, "\"savunma\"");
        *trolSaglik = extractIntValue(trolStr, "\"saglik\"");
        *trolKritikSans = extractIntValue(trolStr, "\"kritik_sans\"");
    }
}

// Function to parse scenario JSON and extract unit counts and heroes/creatures
void parseScenarioJson(const char* jsonVerisi, long long int *humanUnitCounts, long long int *orcUnitCounts, char *humanHero, char *humanCreature, char *orcHero, char *orcCreature) {
    // Parse Human Empire units
    char* humanStr = strstr(jsonVerisi, "\"insan_imparatorlugu\":");
    if (humanStr != NULL) {
        // Units
        char* birimlerStr = strstr(humanStr, "\"birimler\":");
        if (birimlerStr != NULL) {
            humanUnitCounts[0] = extractLongLongIntValue(birimlerStr, "\"piyadeler\"");
            humanUnitCounts[1] = extractLongLongIntValue(birimlerStr, "\"okcular\"");
            humanUnitCounts[2] = extractLongLongIntValue(birimlerStr, "\"suvariler\"");
            humanUnitCounts[3] = extractLongLongIntValue(birimlerStr, "\"kusatma_makineleri\"");
        }

        // Hero
        extractStringValue(humanStr, "\"kahraman\"", humanHero, 50);

        // Creature
        extractStringValue(humanStr, "\"canavar\"", humanCreature, 50);
    }

    // Parse Orc Legion units
    char* orcStr = strstr(jsonVerisi, "\"ork_legi\":");
    if (orcStr != NULL) {
        // Units
        char* birimlerStr = strstr(orcStr, "\"birimler\":");
        if (birimlerStr != NULL) {
            orcUnitCounts[0] = extractLongLongIntValue(birimlerStr, "\"ork_dovusculeri\"");
            orcUnitCounts[1] = extractLongLongIntValue(birimlerStr, "\"mizrakcilar\"");
            orcUnitCounts[2] = extractLongLongIntValue(birimlerStr, "\"varg_binicileri\"");
            orcUnitCounts[3] = extractLongLongIntValue(birimlerStr, "\"troller\"");
        }

        // Hero
        extractStringValue(orcStr, "\"kahraman\"", orcHero, 50);

        // Creature
        extractStringValue(orcStr, "\"canavar\"", orcCreature, 50);
    }
}


// Function to apply hero effects
void kahramanEtkisiUygula(const char* heroesJsonVerisi, const char* heroName, int isHuman,
    int *piyadeSaldiri, int *piyadeSavunma, int *piyadeKritikSans,
    int *okcuSaldiri, int *okcuSavunma, int *okcuKritikSans,
    int *suvariSaldiri, int *suvariSavunma, int *suvariKritikSans,
    int *kusatmaSaldiri, int *kusatmaSavunma, int *kusatmaKritikSans,
    int *orkSaldiri, int *orkSavunma, int *orkKritikSans,
    int *mizrakciSaldiri, int *mizrakciSavunma, int *mizrakciKritikSans,
    int *vargSaldiri, int *vargSavunma, int *vargKritikSans,
    int *trolSaldiri, int *trolSavunma, int *trolKritikSans) {

    if (strlen(heroName) == 0) return; // No hero to apply

    char heroKey[100];
    sprintf(heroKey, "\"%s\":", heroName);
    char* heroStr = strstr(heroesJsonVerisi, heroKey);
    if (heroStr != NULL) {
        char bonusTuru[20];
        int bonusDegeri;

        extractStringValue(heroStr, "\"bonus_turu\"", bonusTuru, sizeof(bonusTuru));
        bonusDegeri = extractIntValue(heroStr, "\"bonus_degeri\"");

        if (strcmp(bonusTuru, "saldiri") == 0) {
            if (isHuman) {
                if (strcmp(heroName, "Alparslan") == 0) {
                    *piyadeSaldiri = *piyadeSaldiri * (100 + bonusDegeri) / 100;
                } else if (strcmp(heroName, "Fatih_Sultan_Mehmet") == 0) {
                    *kusatmaSaldiri = *kusatmaSaldiri * (100 + bonusDegeri) / 100;
                } else if (strcmp(heroName, "Tugrul_Bey") == 0) {
                    *okcuSaldiri = *okcuSaldiri * (100 + bonusDegeri) / 100;
                }
            } else {
                if (strcmp(heroName, "Goruk_Vahsi") == 0) {
                    *orkSaldiri = *orkSaldiri * (100 + bonusDegeri) / 100;
                }
            }
        } else if (strcmp(bonusTuru, "savunma") == 0) {
            if (isHuman) {
                if (strcmp(heroName, "Mete_Han") == 0) {
                    *okcuSavunma = *okcuSavunma * (100 + bonusDegeri) / 100;
                }
            } else {
                if (strcmp(heroName, "Thruk_Kemikkiran") == 0) {
                    *trolSavunma = *trolSavunma * (100 + bonusDegeri) / 100;
                } else if (strcmp(heroName, "Ugar_Zalim") == 0) {
                    *orkSavunma = *orkSavunma * (100 + bonusDegeri) / 100;
                    *mizrakciSavunma = *mizrakciSavunma * (100 + bonusDegeri) / 100;
                    *vargSavunma = *vargSavunma * (100 + bonusDegeri) / 100;
                    *trolSavunma = *trolSavunma * (100 + bonusDegeri) / 100;
                }
            }
        } else if (strcmp(bonusTuru, "kritik_sans") == 0) {
            if (isHuman) {
                if (strcmp(heroName, "Yavuz_Sultan_Selim") == 0) {
                    *suvariKritikSans += bonusDegeri;
                }
            } else {
                if (strcmp(heroName, "Vrog_Kafakiran") == 0) {
                    *vargKritikSans += bonusDegeri;
                }
            }
        }
    }
}

// Function to apply creature effects
void canavarEtkisiUygula(const char* creaturesJsonVerisi, const char* creatureName, int isHuman,
    int *piyadeSaldiri, int *piyadeSavunma, int *piyadeKritikSans,
    int *okcuSaldiri, int *okcuSavunma, int *okcuKritikSans,
    int *suvariSaldiri, int *suvariSavunma, int *suvariKritikSans,
    int *kusatmaSaldiri, int *kusatmaSavunma, int *kusatmaKritikSans,
    int *orkSaldiri, int *orkSavunma, int *orkKritikSans,
    int *mizrakciSaldiri, int *mizrakciSavunma, int *mizrakciKritikSans,
    int *vargSaldiri, int *vargSavunma, int *vargKritikSans,
    int *trolSaldiri, int *trolSavunma, int *trolKritikSans) {

    if (strlen(creatureName) == 0) return; // No creature to apply

    char creatureKey[100];
    sprintf(creatureKey, "\"%s\":", creatureName);
    char* creatureStr = strstr(creaturesJsonVerisi, creatureKey);
    if (creatureStr != NULL) {
        char etkiTuru[20];
        int etkiDegeri;

        extractStringValue(creatureStr, "\"etki_turu\"", etkiTuru, sizeof(etkiTuru));
        etkiDegeri = extractIntValue(creatureStr, "\"etki_degeri\"");

        if (strcmp(etkiTuru, "saldiri") == 0) {
            if (isHuman) {
                if (strcmp(creatureName, "Ejderha") == 0) {
                    *piyadeSaldiri = *piyadeSaldiri * (100 + etkiDegeri) / 100;
                    *okcuSaldiri = *okcuSaldiri * (100 + etkiDegeri) / 100;
                    *suvariSaldiri = *suvariSaldiri * (100 + etkiDegeri) / 100;
                    *kusatmaSaldiri = *kusatmaSaldiri * (100 + etkiDegeri) / 100;
                } else if (strcmp(creatureName, "Tepegoz") == 0) {
                    *okcuSaldiri = *okcuSaldiri * (100 + etkiDegeri) / 100;
                }
            } else {
                if (strcmp(creatureName, "Kara_Troll") == 0) {
                    *trolSaldiri = *trolSaldiri * (100 + etkiDegeri) / 100;
                }
            }
        } else if (strcmp(etkiTuru, "savunma") == 0) {
            if (isHuman) {
                if (strcmp(creatureName, "Karakurt") == 0) {
                    *okcuSavunma = *okcuSavunma * (100 + etkiDegeri) / 100;
                }
            } else {
                if (strcmp(creatureName, "Golge_Kurtlari") == 0) {
                    *vargSavunma = *vargSavunma * (100 + etkiDegeri) / 100;
                }
            }
        } else if (strcmp(etkiTuru, "kritik_sans") == 0) {
            if (isHuman) {
                // For example, "Ejderha" increases critical chance
                if (strcmp(creatureName, "Ejderha") == 0) {
                    *suvariKritikSans += etkiDegeri;
                }
            } else {
                if (strcmp(creatureName, "Kara_Troll") == 0) {
                    *trolKritikSans += etkiDegeri;
                }
            }
        }
    }
}

// Function to apply research effects
void arastirmaEtkisiUygula(const char* researchJsonVerisi, int savunmaUstaligiSeviye, int saldiriGelistirmesiSeviye,
    int *piyadeSaldiri, int *piyadeSavunma,
    int *okcuSaldiri, int *okcuSavunma,
    int *suvariSaldiri, int *suvariSavunma,
    int *kusatmaSaldiri, int *kusatmaSavunma,
    int *orkSaldiri, int *orkSavunma,
    int *mizrakciSaldiri, int *mizrakciSavunma,
    int *vargSaldiri, int *vargSavunma,
    int *trolSaldiri, int *trolSavunma) {

    // Apply defense mastery
    if (savunmaUstaligiSeviye > 0) {
        char key[50];
        sprintf(key, "\"savunma_ustaligi\":");
        char* savunmaStr = strstr(researchJsonVerisi, key);
        if (savunmaStr != NULL) {
            int bonusDegeri = extractIntValue(savunmaStr, "\"deger\"");
            *piyadeSavunma = *piyadeSavunma * (100 + bonusDegeri) / 100;
            *okcuSavunma = *okcuSavunma * (100 + bonusDegeri) / 100;
            *suvariSavunma = *suvariSavunma * (100 + bonusDegeri) / 100;
            *kusatmaSavunma = *kusatmaSavunma * (100 + bonusDegeri) / 100;
        }
    }

    // Apply attack development
    if (saldiriGelistirmesiSeviye > 0) {
        char key[50];
        sprintf(key, "\"saldiri_gelistirmesi\":");
        char* saldiriStr = strstr(researchJsonVerisi, key);
        if (saldiriStr != NULL) {
            int bonusDegeri = extractIntValue(saldiriStr, "\"deger\"");
            *orkSaldiri = *orkSaldiri * (100 + bonusDegeri) / 100;
            *mizrakciSaldiri = *mizrakciSaldiri * (100 + bonusDegeri) / 100;
            *vargSaldiri = *vargSaldiri * (100 + bonusDegeri) / 100;
            *trolSaldiri = *trolSaldiri * (100 + bonusDegeri) / 100;
        }
    }
}

// Structure to hold unit information
typedef struct {
    char isim[50];
    int saldiri;
    int savunma;
    int saglik;
    int maksimumSaglik;
    int kritikSans;
    long long int kalanBirimSayisi;
    Color color; // Birim rengi
    Texture2D texture; // Texture for visualization
    bool hasHeroEffect; // Kahraman etkisi var mý
    bool hasMonsterEffect; // Canavar etkisi var mý
} Birim;

// Function prototypes for visualization
void drawGrid(int cellSize, int rows, int cols);
Vector2 getBirimPosition(int rowIndex, int colIndex, int cellSize);
void loadOrkTextures(Birim *orkLegionu, int count);
void unloadOrkTextures(Birim *orkLegionu, int count);
void loadInsanTextures(Birim *insanImparatorlugu, int count);
void unloadInsanTextures(Birim *insanImparatorlugu, int count);
void drawHealthBar(Vector2 position, int currentHealth, int maxHealth, int cellSize, bool hasHeroEffect, bool hasMonsterEffect);
void drawBirimCount(Vector2 position, long long int unitCount);
void placeUnitsInGrid(Birim *birimler, int birimCount, int cellSize, int startRow, int startCol);

// Function to simulate one battle round with critical hits
// Simülasyon fonksiyonunu güncelle
// Function to simulate one battle round with scheduled critical hits
void simulateRound(Birim *insanImparatorlugu, int insanUnitCount, Birim *orkLegionu, int orkUnitCount, FILE *logFile, int round,int roundNumber,
                  int *attackCountHuman, int *critThresholdHuman,
                  int *attackCountOrc, int *critThresholdOrc) {
    fprintf(logFile, "\n--- Round %d ---\n",round);

    // Yorgunluk yalnýzca her fatigueFrequency turda bir uygulanýr
    if (round % 5 == 0) {
        float fatiguePercentage = 0.10f; // %10 yorgunluk oraný
        for (int i = 0; i < insanUnitCount; i++) {
            applyFatigueEffect(&insanImparatorlugu[i].saldiri, &insanImparatorlugu[i].savunma, fatiguePercentage);
        }
        for (int i = 0; i < orkUnitCount; i++) {
            applyFatigueEffect(&orkLegionu[i].saldiri, &orkLegionu[i].savunma, fatiguePercentage);
        }
        fprintf(logFile, "Yorgunluk devreye girdi: Tur %11d, birimlerin saldýrý ve savunma güçleri %10 azaldý.\n", round);
    }

    // Orc birimlerine saldýrmak için bir hedef indeksi tut
    static int orkAttackIndex = 0;
    static int insanAttackIndex = 0;

    // HUMAN ATTACK
    for (int i = 0; i < insanUnitCount; i++) {
        if (insanImparatorlugu[i].kalanBirimSayisi > 0) {
            // Increment attack count
            attackCountHuman[i]++;
            // Check for critical hit
            bool isCritical = false;
            if (attackCountHuman[i] >= critThresholdHuman[i]) {
                isCritical = true;
                attackCountHuman[i] = 0; // Reset counter after critical hit
            }

            // Calculate attack power
            long long int attackPower = (long long int)insanImparatorlugu[i].saldiri * insanImparatorlugu[i].kalanBirimSayisi;
            if (isCritical) {
                attackPower = (long long int)(attackPower * 1.5); // Increase by 50%
                fprintf(logFile, "Round %d: Human unit (%s) lands a SCHEDULED CRITICAL HIT! Attack power increased by 50%% to %lld.\n", round, insanImparatorlugu[i].isim, attackPower);
            }

            // Hedef Orc birimini seç
            int targetIndex = orkAttackIndex;
            // Find the next available Orc unit
            bool targetFound = false;
            for (int k = 0; k < orkUnitCount; k++) {
                int currentIndex = (orkAttackIndex + k) % orkUnitCount;
                if (orkLegionu[currentIndex].kalanBirimSayisi > 0) {
                    targetIndex = currentIndex;
                    orkAttackIndex = (currentIndex + 1) % orkUnitCount;
                    targetFound = true;
                    break;
                }
            }

            if (targetFound) {
                // Hasar hesaplama
                long long int damage = calculateNetDamage(attackPower, (long long int)orkLegionu[targetIndex].savunma);
                orkLegionu[targetIndex].saglik -= damage;

                // Loglama
                fprintf(logFile, "Human unit (%s) attacks Orc unit (%s) for %lld damage.\n", insanImparatorlugu[i].isim, orkLegionu[targetIndex].isim, damage);

                // Orc biriminin ölmesi durumunda
                if (orkLegionu[targetIndex].saglik <= 0) {
                    orkLegionu[targetIndex].saglik = orkLegionu[targetIndex].maksimumSaglik;
                    orkLegionu[targetIndex].kalanBirimSayisi--;
                    fprintf(logFile, "Orc unit (%s) has been defeated. Remaining units: %lld\n", orkLegionu[targetIndex].isim, orkLegionu[targetIndex].kalanBirimSayisi);
                }
            }
        }
    }

    // ORC ATTACK
    for (int i = 0; i < orkUnitCount; i++) {
        if (orkLegionu[i].kalanBirimSayisi > 0) {
            // Increment attack count
            attackCountOrc[i]++;
            // Check for critical hit
            bool isCritical = false;
            if (attackCountOrc[i] >= critThresholdOrc[i]) {
                isCritical = true;
                attackCountOrc[i] = 0; // Reset counter after critical hit
            }

            // Calculate attack power
            long long int attackPower = (long long int)orkLegionu[i].saldiri * orkLegionu[i].kalanBirimSayisi;
            if (isCritical) {
                attackPower = (long long int)(attackPower * 1.5); // Increase by 50%
                fprintf(logFile, "Round %d: Orc unit (%s) lands a SCHEDULED CRITICAL HIT! Attack power increased by 50%% to %lld.\n", round, orkLegionu[i].isim, attackPower);
            }

            // Hedef Human birimini seç
            int targetIndex = insanAttackIndex;
            // Find the next available Human unit
            bool targetFound = false;
            for (int k = 0; k < insanUnitCount; k++) {
                int currentIndex = (insanAttackIndex + k) % insanUnitCount;
                if (insanImparatorlugu[currentIndex].kalanBirimSayisi > 0) {
                    targetIndex = currentIndex;
                    insanAttackIndex = (currentIndex + 1) % insanUnitCount;
                    targetFound = true;
                    break;
                }
            }

            if (targetFound) {
                // Hasar hesaplama
                long long int damage = calculateNetDamage(attackPower, (long long int)insanImparatorlugu[targetIndex].savunma);
                insanImparatorlugu[targetIndex].saglik -= damage;

                // Loglama
                fprintf(logFile, "Orc unit (%s) attacks Human unit (%s) for %lld damage.\n", orkLegionu[i].isim, insanImparatorlugu[targetIndex].isim, damage);

                // Human biriminin ölmesi durumunda
                if (insanImparatorlugu[targetIndex].saglik <= 0) {
                    insanImparatorlugu[targetIndex].saglik = insanImparatorlugu[targetIndex].maksimumSaglik;
                    insanImparatorlugu[targetIndex].kalanBirimSayisi--;
                    fprintf(logFile, "Human unit (%s) has been defeated. Remaining units: %lld\n", insanImparatorlugu[targetIndex].isim, insanImparatorlugu[targetIndex].kalanBirimSayisi);
                }
            }
        }
    }

    // Mevcut durumu logla
    fprintf(logFile, "Status after Round %d:\n", roundNumber);
    fprintf(logFile, "Humans:\n");
    for (int i = 0; i < insanUnitCount; i++) {
        fprintf(logFile, " - %s: %lld units remaining, Health per unit: %d\n", insanImparatorlugu[i].isim, insanImparatorlugu[i].kalanBirimSayisi, insanImparatorlugu[i].saglik);
    }

    fprintf(logFile, "Orcs:\n");
    for (int i = 0; i < orkUnitCount; i++) {
        fprintf(logFile, " - %s: %lld units remaining, Health per unit: %d\n", orkLegionu[i].isim, orkLegionu[i].kalanBirimSayisi, orkLegionu[i].saglik);
    }

    fprintf(logFile, "----------------------------------------\n");
}



// Function to select and download the scenario based on user's choice
const char* selectScenario() {
    int choice;
    printf("Please choose a scenario (1-10):\n");
    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "Invalid input. Defaulting to Scenario 1.\n");
        return "https://yapbenzet.org.tr/1.json";
    }

    switch(choice) {
        case 1: return "https://yapbenzet.org.tr/1.json";
        case 2: return "https://yapbenzet.org.tr/2.json";
        case 3: return "https://yapbenzet.org.tr/3.json";
        case 4: return "https://yapbenzet.org.tr/4.json";
        case 5: return "https://yapbenzet.org.tr/5.json";
        case 6: return "https://yapbenzet.org.tr/6.json";
        case 7: return "https://yapbenzet.org.tr/7.json";
        case 8: return "https://yapbenzet.org.tr/8.json";
        case 9: return "https://yapbenzet.org.tr/9.json";
        case 10:return "https://yapbenzet.org.tr/10.json";
        default:
            printf("Invalid choice, defaulting to Scenario 1.\n");
            return "https://yapbenzet.org.tr/1.json";
    }
}

// Function to draw the grid
void drawGrid(int cellSize, int rows, int cols) {
    for (int i = 0; i <= cols; i++) {
        DrawLine(i * cellSize, 0, i * cellSize, GetScreenHeight(), LIGHTGRAY);
    }
    for (int j = 0; j <= rows; j++) {
        DrawLine(0, j * cellSize, GetScreenWidth(), j * cellSize, LIGHTGRAY);
    }
}

// Function to get the position of a unit in the grid
Vector2 getBirimPosition(int rowIndex, int colIndex, int cellSize) {
    int x = colIndex * cellSize;
    int y = rowIndex * cellSize;
    return (Vector2){x, y};
}

// Function to load Orc textures
void loadOrkTextures(Birim *orkLegionu, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(orkLegionu[i].isim, "Ork Dövüþçüleri") == 0) {
            orkLegionu[i].texture = LoadTexture("C:\\jsons2\\orklar.png");
        } else if (strcmp(orkLegionu[i].isim, "Mýzrakçýlar") == 0) {
            orkLegionu[i].texture = LoadTexture("C:\\jsons2\\mizrakci.png");
        } else if (strcmp(orkLegionu[i].isim, "Varg Binicileri") == 0) {
            orkLegionu[i].texture = LoadTexture("C:\\jsons2\\varg.png");
        } else if (strcmp(orkLegionu[i].isim, "Troller") == 0) {
            orkLegionu[i].texture = LoadTexture("C:\\jsons2\\troll.png");
        } else {
            orkLegionu[i].texture = LoadTexture("C:\\jsons2\\default.png");
        }
    }
}

// Function to unload Orc textures
void unloadOrkTextures(Birim *orkLegionu, int count) {
    for (int i = 0; i < count; i++) {
        UnloadTexture(orkLegionu[i].texture);
    }
}

// Function to load Human textures
void loadInsanTextures(Birim *insanImparatorlugu, int count) {
    for (int i = 0; i < count; i++) {
        if (strcmp(insanImparatorlugu[i].isim, "Piyadeler") == 0) {
            insanImparatorlugu[i].texture = LoadTexture("C:\\jsons2\\piyade.png");
        } else if (strcmp(insanImparatorlugu[i].isim, "Okçular") == 0) {
            insanImparatorlugu[i].texture = LoadTexture("C:\\jsons2\\okcu.png");
        } else if (strcmp(insanImparatorlugu[i].isim, "Süvariler") == 0) {
            insanImparatorlugu[i].texture = LoadTexture("C:\\jsons2\\suvari.png");
        } else if (strcmp(insanImparatorlugu[i].isim, "Kuþatma Makineleri") == 0) {
            insanImparatorlugu[i].texture = LoadTexture("C:\\jsons2\\kusatma.png");
        } else {
            insanImparatorlugu[i].texture = LoadTexture("C:\\jsons2\\default.png");
        }
    }
}

// Function to unload Human textures
void unloadInsanTextures(Birim *insanImparatorlugu, int count) {
    for (int i = 0; i < count; i++) {
        UnloadTexture(insanImparatorlugu[i].texture);
    }
}

// Function to draw health bar
void drawHealthBar(Vector2 position, int currentHealth, int maxHealth, int cellSize, bool hasHeroEffect, bool hasMonsterEffect) {
    int barWidth = cellSize - 5;
    int barHeight = 8;
    float healthPercentage = (float)currentHealth / maxHealth;

    // Normal bar color
    Color barColor = (healthPercentage > 0.8) ? GREEN : (healthPercentage > 0.2 ? YELLOW : RED);

    // Highlight if has hero or monster effect
    if (hasHeroEffect || hasMonsterEffect) {
        if (healthPercentage > 0.8) {
            barColor = LIME; // Bright green
        } else if (healthPercentage > 0.2) {
            barColor = GOLD; // Bright yellow
        } else {
            barColor = MAROON; // Bright red
        }
    }

    DrawRectangle(position.x, position.y - 9, barWidth * healthPercentage, barHeight, barColor);
}

// Function to draw unit count
void drawBirimCount(Vector2 position, long long int unitCount) {
    char buffer[20];
    sprintf(buffer, "%lld", unitCount); // Convert unit count to string
    DrawText(buffer, position.x + 9, position.y - 9, 10, BLACK); // Centered position
}

// Function to place units in the grid
void placeUnitsInGrid(Birim *birimler, int birimCount, int cellSize, int startRow, int startCol) {
    for (int i = 0; i < birimCount; i++) {
        for (int j = 0; j < 18; j++) {
            int rowOffset = j / 3;  // 3 sütuna yay
            int colOffset = j % 3;  // Her bir satýrda 3 hücre geniþliðinde olacak

            Vector2 pozisyon = getBirimPosition(startRow + rowOffset, startCol + colOffset + (i * 4), cellSize);

            // Texture için kaynak ve hedef dikdörtgenleri belirle
            Rectangle source = { 0, 0, (float)birimler[i].texture.width, (float)birimler[i].texture.height };
            Rectangle dest = { pozisyon.x, pozisyon.y, (float)cellSize, (float)cellSize }; // Hücre boyutuna sýðacak

            // Texture'u hücre boyutuna göre yeniden boyutlandýrarak çiz
            DrawTexturePro(birimler[i].texture, source, dest, (Vector2){ 0, 0 }, 0.0f, WHITE);

            // Saðlýk barýný çiz
            drawHealthBar(pozisyon, birimler[i].saglik, birimler[i].maksimumSaglik, cellSize, birimler[i].hasHeroEffect, birimler[i].hasMonsterEffect);
            // Birim sayýsýný göster
            drawBirimCount(pozisyon, birimler[i].kalanBirimSayisi);
        }
    }
}

int main() {
    // Seed the random number generator
    srand(time(NULL));
    int orkAttackIndex = 0;
    int insanAttackIndex = 0;
    // Initialize cURL
    curl_global_init(CURL_GLOBAL_ALL);

    // Open the log file
    FILE *logFile = fopen("savas_sim.txt", "w");
    if (!logFile) {
        fprintf(stderr, "Failed to open log file.\n");
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    // Select and download the scenario
    const char* scenarioUrl = selectScenario();
    const char* output_file = "selected_scenario.json";

    // Download the selected scenario
    if (download_json(scenarioUrl, output_file) != 0) {
        fprintf(stderr, "Failed to download the scenario.\n");
        fclose(logFile);
        curl_global_cleanup();
        return 1;
    }

    // Paths to JSON files
    const char* unitTypesFilePath = "C:\\json\\unit_types.json";
    const char* heroesFilePath = "C:\\json\\heroes.json";
    const char* creaturesFilePath = "C:\\json\\creatures.json";
    const char* researchFilePath = "C:\\json\\research.json";
    const char* scenarioFilePath = "selected_scenario.json";

    // Read JSON files
    char* unitTypesJson = readJsonFromFile(unitTypesFilePath);
    if (unitTypesJson == NULL) {
        fprintf(stderr, "Failed to read unit types JSON.\n");
        fclose(logFile);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char* heroesJson = readJsonFromFile(heroesFilePath);
    if (heroesJson == NULL) {
        fprintf(stderr, "Failed to read heroes JSON.\n");
        free(unitTypesJson);
        fclose(logFile);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char* creaturesJson = readJsonFromFile(creaturesFilePath);
    if (creaturesJson == NULL) {
        fprintf(stderr, "Failed to read creatures JSON.\n");
        free(unitTypesJson);
        free(heroesJson);
        fclose(logFile);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char* researchJson = readJsonFromFile(researchFilePath);
    if (researchJson == NULL) {
        fprintf(stderr, "Failed to read research JSON.\n");
        free(unitTypesJson);
        free(heroesJson);
        free(creaturesJson);
        fclose(logFile);
        curl_global_cleanup();
        return EXIT_FAILURE;
    }

    char* scenarioJson = readJsonFromFile(scenarioFilePath);
    if (scenarioJson == NULL) {
        fprintf(stderr, "Failed to read scenario JSON.\n");
        free(unitTypesJson);
        free(heroesJson);
        free(creaturesJson);
        free(researchJson);
        fclose(logFile);
        curl_global_cleanup();
        return EXIT_FAILURE;
    } else {
        printf("Scenario JSON loaded successfully.\n");
    }

    // Set up unit attributes for all units
    int piyadeSaldiri = 0, piyadeSavunma = 0, piyadeSaglik = 0, piyadeKritikSans = 0;
    int okcuSaldiri = 0, okcuSavunma = 0, okcuSaglik = 0, okcuKritikSans = 0;
    int suvariSaldiri = 0, suvariSavunma = 0, suvariSaglik = 0, suvariKritikSans = 0;
    int kusatmaSaldiri = 0, kusatmaSavunma = 0, kusatmaSaglik = 0, kusatmaKritikSans = 0;
    int orkSaldiri = 0, orkSavunma = 0, orkSaglik = 0, orkKritikSans = 0;
    int mizrakciSaldiri = 0, mizrakciSavunma = 0, mizrakciSaglik = 0, mizrakciKritikSans = 0;
    int vargSaldiri = 0, vargSavunma = 0, vargSaglik = 0, vargKritikSans = 0;
    int trolSaldiri = 0, trolSavunma = 0, trolSaglik = 0, trolKritikSans = 0;

    jsonVerisiniIsleVeBirimOzellikleriniAyarla(unitTypesJson,
        &piyadeSaldiri, &piyadeSavunma, &piyadeSaglik, &piyadeKritikSans,
        &okcuSaldiri, &okcuSavunma, &okcuSaglik, &okcuKritikSans,
        &suvariSaldiri, &suvariSavunma, &suvariSaglik, &suvariKritikSans,
        &kusatmaSaldiri, &kusatmaSavunma, &kusatmaSaglik, &kusatmaKritikSans,
        &orkSaldiri, &orkSavunma, &orkSaglik, &orkKritikSans,
        &mizrakciSaldiri, &mizrakciSavunma, &mizrakciSaglik, &mizrakciKritikSans,
        &vargSaldiri, &vargSavunma, &vargSaglik, &vargKritikSans,
        &trolSaldiri, &trolSavunma, &trolSaglik, &trolKritikSans);

    // Arrays to hold unit counts
    long long int humanUnitCounts[4] = {0}; // Piyadeler, Okçular, Süvariler, Kuþatma Makineleri
    long long int orcUnitCounts[4] = {0};   // Ork Dövüþçüleri, Mýzrakçýlar, Varg Binicileri, Troller

    // Hero and creature names
    char humanHero[50] = {0}, humanCreature[50] = {0};
    char orcHero[50] = {0}, orcCreature[50] = {0};

    parseScenarioJson(scenarioJson, humanUnitCounts, orcUnitCounts, humanHero, humanCreature, orcHero, orcCreature);

    // Apply hero effects
    kahramanEtkisiUygula(heroesJson, humanHero, 1,
        &piyadeSaldiri, &piyadeSavunma, &piyadeKritikSans,
        &okcuSaldiri, &okcuSavunma, &okcuKritikSans,
        &suvariSaldiri, &suvariSavunma, &suvariKritikSans,
        &kusatmaSaldiri, &kusatmaSavunma, &kusatmaKritikSans,
        &orkSaldiri, &orkSavunma, &orkKritikSans,
        &mizrakciSaldiri, &mizrakciSavunma, &mizrakciKritikSans,
        &vargSaldiri, &vargSavunma, &vargKritikSans,
        &trolSaldiri, &trolSavunma, &trolKritikSans);

    kahramanEtkisiUygula(heroesJson, orcHero, 0,
        &piyadeSaldiri, &piyadeSavunma, &piyadeKritikSans,
        &okcuSaldiri, &okcuSavunma, &okcuKritikSans,
        &suvariSaldiri, &suvariSavunma, &suvariKritikSans,
        &kusatmaSaldiri, &kusatmaSavunma, &kusatmaKritikSans,
        &orkSaldiri, &orkSavunma, &orkKritikSans,
        &mizrakciSaldiri, &mizrakciSavunma, &mizrakciKritikSans,
        &vargSaldiri, &vargSavunma, &vargKritikSans,
        &trolSaldiri, &trolSavunma, &trolKritikSans);

    // Apply creature effects
    canavarEtkisiUygula(creaturesJson, humanCreature, 1,
        &piyadeSaldiri, &piyadeSavunma, &piyadeKritikSans,
        &okcuSaldiri, &okcuSavunma, &okcuKritikSans,
        &suvariSaldiri, &suvariSavunma, &suvariKritikSans,
        &kusatmaSaldiri, &kusatmaSavunma, &kusatmaKritikSans,
        &orkSaldiri, &orkSavunma, &orkKritikSans,
        &mizrakciSaldiri, &mizrakciSavunma, &mizrakciKritikSans,
        &vargSaldiri, &vargSavunma, &vargKritikSans,
        &trolSaldiri, &trolSavunma, &trolKritikSans);

    canavarEtkisiUygula(creaturesJson, orcCreature, 0,
        &piyadeSaldiri, &piyadeSavunma, &piyadeKritikSans,
        &okcuSaldiri, &okcuSavunma, &okcuKritikSans,
        &suvariSaldiri, &suvariSavunma, &suvariKritikSans,
        &kusatmaSaldiri, &kusatmaSavunma, &kusatmaKritikSans,
        &orkSaldiri, &orkSavunma, &orkKritikSans,
        &mizrakciSaldiri, &mizrakciSavunma, &mizrakciKritikSans,
        &vargSaldiri, &vargSavunma, &vargKritikSans,
        &trolSaldiri, &trolSavunma, &trolKritikSans);

    // Apply research effects
    int humanDefenseMasteryLevel = extractIntValue(scenarioJson, "\"savunma_ustaligi\"");
    int orcAttackDevelopmentLevel = extractIntValue(scenarioJson, "\"saldiri_gelistirmesi\"");

    arastirmaEtkisiUygula(researchJson, humanDefenseMasteryLevel, orcAttackDevelopmentLevel,
        &piyadeSaldiri, &piyadeSavunma,
        &okcuSaldiri, &okcuSavunma,
        &suvariSaldiri, &suvariSavunma,
        &kusatmaSaldiri, &kusatmaSavunma,
        &orkSaldiri, &orkSavunma,
        &mizrakciSaldiri, &mizrakciSavunma,
        &vargSaldiri, &vargSavunma,
        &trolSaldiri, &trolSavunma);

    // Initialize unit health and counts
    Birim insanImparatorlugu[4] = {
        {"Piyadeler", piyadeSaldiri, piyadeSavunma, piyadeSaglik, piyadeSaglik, piyadeKritikSans, humanUnitCounts[0], ORANGE, {0}},
        {"Okçular", okcuSaldiri, okcuSavunma, okcuSaglik, okcuSaglik, okcuKritikSans, humanUnitCounts[1], DARKBLUE, {0}},
        {"Süvariler", suvariSaldiri, suvariSavunma, suvariSaglik, suvariSaglik, suvariKritikSans, humanUnitCounts[2], RED, {0}},
        {"Kuþatma Makineleri", kusatmaSaldiri, kusatmaSavunma, kusatmaSaglik, kusatmaSaglik, kusatmaKritikSans, humanUnitCounts[3], DARKGREEN, {0}}
    };
    int insanUnitCount = 4;

    Birim orkLegionu[4] = {
        {"Ork Dövüþçüleri", orkSaldiri, orkSavunma, orkSaglik, orkSaglik, orkKritikSans, orcUnitCounts[0], DARKGRAY, {0}},
        {"Mýzrakçýlar", mizrakciSaldiri, mizrakciSavunma, mizrakciSaglik, mizrakciSaglik, mizrakciKritikSans, orcUnitCounts[1], MAROON, {0}},
        {"Varg Binicileri", vargSaldiri, vargSavunma, vargSaglik, vargSaglik, vargKritikSans, orcUnitCounts[2], BROWN, {0}},
        {"Troller", trolSaldiri, trolSavunma, trolSaglik, trolSaglik, trolKritikSans, orcUnitCounts[3], DARKBLUE, {0}}
    };
    int orkUnitCount = 4;

    // Initialize Raylib
    const int ekranGenisligi = 800;
    const int ekranYuksekligi = 800;
    InitWindow(ekranGenisligi, ekranYuksekligi, "Savaþ Simülasyonu");

    // Set target FPS
    SetTargetFPS(60);

    // Load textures AFTER initializing Raylib
    loadInsanTextures(insanImparatorlugu, insanUnitCount);
    loadOrkTextures(orkLegionu, orkUnitCount);

    // Initialize attack count arrays
    int attackCountHuman[4] = {0}; // For Piyadeler, Okçular, Süvariler, Kuþatma Makineleri
    int critThresholdHuman[4];
    for(int i = 0; i < 4; i++) {
        if(insanImparatorlugu[i].kritikSans > 0) {
            critThresholdHuman[i] = (int)(100.0 / insanImparatorlugu[i].kritikSans);
            if(critThresholdHuman[i] == 0) critThresholdHuman[i] = INT_MAX; // Prevent division by zero
        } else {
            critThresholdHuman[i] = INT_MAX; // No critical hits
        }
    }

    int attackCountOrc[4] = {0}; // For Ork Dövüþçüleri, Mýzrakçýlar, Varg Binicileri, Troller
    int critThresholdOrc[4];
    for(int i = 0; i < 4; i++) {
        if(orkLegionu[i].kritikSans > 0) {
            critThresholdOrc[i] = (int)(100.0 / orkLegionu[i].kritikSans);
            if(critThresholdOrc[i] == 0) critThresholdOrc[i] = INT_MAX; // Prevent division by zero
        } else {
            critThresholdOrc[i] = INT_MAX; // No critical hits
        }
    }

    // Initialize simulation variables
    int roundNumber = 1;
    bool battleOngoing = true;
    float fatiguePercentage = 0.10f; // Fatigue percentage
    int fatigueFrequency = 5;        // Apply fatigue every 'fatigueFrequency' rounds
    int maxRounds = 10000;           // Maximum number of rounds

    fprintf(logFile, "\nBattle Start!\n");

    // Savaþ simülasyonunu baþlat
    while (!WindowShouldClose() && battleOngoing && roundNumber <= maxRounds) {
        BeginDrawing();
        ClearBackground(RAYWHITE);

        // Izgarayý çiz
        const int cellSize = 40; // Cell size
        const int rows = 20;
        const int cols = 20;
        drawGrid(cellSize, rows, cols);

        // Birimleri yerleþtir
        // Ýnsan birimlerini yerleþtir (üst tarafta, sol)
        placeUnitsInGrid(insanImparatorlugu, insanUnitCount, cellSize, 1, 1);

        // Ork birimlerini yerleþtir (alt tarafta, sað)
        placeUnitsInGrid(orkLegionu, orkUnitCount, cellSize, 13, 1);

        EndDrawing();

        // Log and simulate the battle round
        fprintf(logFile, "\n--- Round %d ---\n", roundNumber);

        // Apply fatigue every 'fatigueFrequency' rounds
        if (roundNumber % fatigueFrequency == 0) {
            for (int i = 0; i < insanUnitCount; i++) {
                applyFatigueEffect(&insanImparatorlugu[i].saldiri, &insanImparatorlugu[i].savunma, fatiguePercentage);
            }
            for (int i = 0; i < orkUnitCount; i++) {
                applyFatigueEffect(&orkLegionu[i].saldiri, &orkLegionu[i].savunma, fatiguePercentage);
            }
            fprintf(logFile, "Yorgunluk devreye girdi: Tur %d, birimlerin saldýrý ve savunma güçleri %%10 azaldý.\n", roundNumber);
        }

        // Simulate the battle round
        // HUMAN ATTACK
        for (int i = 0; i < insanUnitCount; i++) {
            if (insanImparatorlugu[i].kalanBirimSayisi > 0) {
                // Increment attack count
                attackCountHuman[i]++;
                // Check for critical hit
                bool isCritical = false;
                if (attackCountHuman[i] >= critThresholdHuman[i]) {
                    isCritical = true;
                    attackCountHuman[i] = 0; // Reset counter after critical hit
                }

                // Calculate attack power
                long long int attackPower = (long long int)insanImparatorlugu[i].saldiri * insanImparatorlugu[i].kalanBirimSayisi;
                if (isCritical) {
                    attackPower = (long long int)(attackPower * 1.5); // Increase by 50%
                    fprintf(logFile, "Round %d: Human unit (%s) lands a SCHEDULED CRITICAL HIT! Attack power increased by 50%% to %lld.\n", roundNumber, insanImparatorlugu[i].isim, attackPower);
                }

                // Hedef Orc birimini seç
                int targetIndex = orkAttackIndex;
                // Find the next available Orc unit
                bool targetFound = false;
                for (int k = 0; k < orkUnitCount; k++) {
                    int currentIndex = (orkAttackIndex + k) % orkUnitCount;
                    if (orkLegionu[currentIndex].kalanBirimSayisi > 0) {
                        targetIndex = currentIndex;
                        orkAttackIndex = (currentIndex + 1) % orkUnitCount;
                        targetFound = true;
                        break;
                    }
                }

                if (targetFound) {
                    // Hasar hesaplama
                    long long int damage = calculateNetDamage(attackPower, (long long int)orkLegionu[targetIndex].savunma);
                    orkLegionu[targetIndex].saglik -= damage;

                    // Loglama
                    fprintf(logFile, "Human unit (%s) attacks Orc unit (%s) for %lld damage.\n", insanImparatorlugu[i].isim, orkLegionu[targetIndex].isim, damage);

                    // Orc biriminin ölmesi durumunda
                    if (orkLegionu[targetIndex].saglik <= 0) {
                        orkLegionu[targetIndex].saglik = orkLegionu[targetIndex].maksimumSaglik;
                        orkLegionu[targetIndex].kalanBirimSayisi--;
                        fprintf(logFile, "Orc unit (%s) has been defeated. Remaining units: %lld\n", orkLegionu[targetIndex].isim, orkLegionu[targetIndex].kalanBirimSayisi);
                    }
                }
            }
        }

        // ORC ATTACK
        for (int i = 0; i < orkUnitCount; i++) {
            if (orkLegionu[i].kalanBirimSayisi > 0) {
                // Increment attack count
                attackCountOrc[i]++;
                // Check for critical hit
                bool isCritical = false;
                if (attackCountOrc[i] >= critThresholdOrc[i]) {
                    isCritical = true;
                    attackCountOrc[i] = 0; // Reset counter after critical hit
                }

                // Calculate attack power
                long long int attackPower = (long long int)orkLegionu[i].saldiri * orkLegionu[i].kalanBirimSayisi;
                if (isCritical) {
                    attackPower = (long long int)(attackPower * 1.5); // Increase by 50%
                    fprintf(logFile, "Round %d: Orc unit (%s) lands a SCHEDULED CRITICAL HIT! Attack power increased by 50%% to %lld.\n", roundNumber, orkLegionu[i].isim, attackPower);
                }

                // Hedef Human birimini seç
                int targetIndex = insanAttackIndex;
                // Find the next available Human unit
                bool targetFound = false;
                for (int k = 0; k < insanUnitCount; k++) {
                    int currentIndex = (insanAttackIndex + k) % insanUnitCount;
                    if (insanImparatorlugu[currentIndex].kalanBirimSayisi > 0) {
                        targetIndex = currentIndex;
                        insanAttackIndex = (currentIndex + 1) % insanUnitCount;
                        targetFound = true;
                        break;
                    }
                }

                if (targetFound) {
                    // Hasar hesaplama
                    long long int damage = calculateNetDamage(attackPower, (long long int)insanImparatorlugu[targetIndex].savunma);
                    insanImparatorlugu[targetIndex].saglik -= damage;

                    // Loglama
                    fprintf(logFile, "Orc unit (%s) attacks Human unit (%s) for %lld damage.\n", orkLegionu[i].isim, insanImparatorlugu[targetIndex].isim, damage);

                    // Human biriminin ölmesi durumunda
                    if (insanImparatorlugu[targetIndex].saglik <= 0) {
                        insanImparatorlugu[targetIndex].saglik = insanImparatorlugu[targetIndex].maksimumSaglik;
                        insanImparatorlugu[targetIndex].kalanBirimSayisi--;
                        fprintf(logFile, "Human unit (%s) has been defeated. Remaining units: %lld\n", insanImparatorlugu[targetIndex].isim, insanImparatorlugu[targetIndex].kalanBirimSayisi);
                    }
                }
            }
        }

        // Mevcut durumu logla
        fprintf(logFile, "Status after Round %d:\n", roundNumber);
        fprintf(logFile, "Humans:\n");
        for (int i = 0; i < insanUnitCount; i++) {
            fprintf(logFile, " - %s: %lld units remaining, Health per unit: %d\n", insanImparatorlugu[i].isim, insanImparatorlugu[i].kalanBirimSayisi, insanImparatorlugu[i].saglik);
        }

        fprintf(logFile, "Orcs:\n");
        for (int i = 0; i < orkUnitCount; i++) {
            fprintf(logFile, " - %s: %lld units remaining, Health per unit: %d\n", orkLegionu[i].isim, orkLegionu[i].kalanBirimSayisi, orkLegionu[i].saglik);
        }

        fprintf(logFile, "----------------------------------------\n");

        // Check if battle has ended
        bool insanKaybetti = true;
        for (int i = 0; i < insanUnitCount; i++) {
            if (insanImparatorlugu[i].kalanBirimSayisi > 0) {
                insanKaybetti = false;
                break;
            }
        }

        bool orkKaybetti = true;
        for (int i = 0; i < orkUnitCount; i++) {
            if (orkLegionu[i].kalanBirimSayisi > 0) {
                orkKaybetti = false;
                break;
            }
        }

        if (insanKaybetti || orkKaybetti) {
            // Battle has ended, determine winner
            if (insanKaybetti && orkKaybetti) {
                fprintf(logFile, "\nBattle ended on round %d.\nIt's a draw!\n", roundNumber);
            }
            else if (insanKaybetti) {
                fprintf(logFile, "\nBattle ended on round %d.\nOrcs win!\n", roundNumber);
            }
            else {
                fprintf(logFile, "\nBattle ended on round %d.\nHumans win!\n", roundNumber);
            }
            battleOngoing = false;
        }

        // Check for maximum rounds to prevent infinite loops
        if (roundNumber >= maxRounds) {
            // Determine the winner based on total remaining units
            long long int totalHumanUnits = 0;
            for (int i = 0; i < insanUnitCount; i++) {
                totalHumanUnits += insanImparatorlugu[i].kalanBirimSayisi;
            }

            long long int totalOrcUnits = 0;
            for (int i = 0; i < orkUnitCount; i++) {
                totalOrcUnits += orkLegionu[i].kalanBirimSayisi;
            }

            if (totalHumanUnits > totalOrcUnits) {
                fprintf(logFile, "\nBattle ended after %d rounds.\nHumans win by remaining units!\n", roundNumber);
            } else if (totalOrcUnits > totalHumanUnits) {
                fprintf(logFile, "\nBattle ended after %d rounds.\nOrcs win by remaining units!\n", roundNumber);
            } else {
                fprintf(logFile, "\nBattle ended in a draw after %d rounds.\n", roundNumber);
            }
            break;
        }

        roundNumber++;
    }

    // Clean up allocated memory
    free(unitTypesJson);
    free(heroesJson);
    free(creaturesJson);
    free(researchJson);
    free(scenarioJson);

    // Unload textures
    unloadInsanTextures(insanImparatorlugu, insanUnitCount);
    unloadOrkTextures(orkLegionu, orkUnitCount);

    fclose(logFile);
    curl_global_cleanup();

    printf("Battle simulation completed. Check 'savas_sim.txt' for details.\n");

    // Keep the window open until closed
    while (!WindowShouldClose()) {
        BeginDrawing();

        // Optionally, you can display additional information here
        DrawText("Savas tamamlandi. Detaylar için 'savas_sim.txt' dosyasina bakin.", 100, ekranYuksekligi / 2, 20, RED);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
