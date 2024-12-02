#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAIN_ENTITY_POINTER 0
#define EMPTY_POINTER -1
#define DATA_BLOCK_SIZE 50

// ESTRUCTURAS
typedef struct Entity {
    char name[DATA_BLOCK_SIZE];
    long dataPointer;
    long attributesPointer;
    long nextEntity;
} ENTITY;

typedef struct Attribute {
    char name[DATA_BLOCK_SIZE];
    bool isPrimary;
    long type;
    long size;
    long nextAttribute;
} ATTRIBUTE;

// PROTOTYPES
FILE* initializeDataDictionary(const char *dictionaryName);
int appendEntity(FILE* dataDictionary, ENTITY newEntity);
void reorderEntities(FILE* dataDictionary, long currentEntityPointer, const char* newEntityName, long newEntityDirection);
void createEntity(FILE* dataDictionary);
ENTITY removeEntity(FILE* dataDictionary, long currentEntityPointer, const char* entityName);
void deleteEntity(FILE* dataDictionary); // Nueva función
void captureEntities(FILE* dataDictionary);
void showEntities(FILE* dataDictionary);
void searchEntityByName(FILE* dataDictionary, const char* entityName, ENTITY* resultEntity);
void askNameEntity(FILE* dataDictionary, ENTITY* resultEntity);

// ATTRIBUTES
int appendAttribute(FILE* dataDictionary, ATTRIBUTE newAttribute);
void reorderAttributes(FILE* dataDictionary, long currentAttributePointer, const char* newAttributeName, long newAttributeDirection);
void createAttribute(FILE* dataDictionary, ENTITY currentEntity);
ATTRIBUTE removeAttribute(FILE* dataDictionary, long currentAttributePointer, const char* attributeName, ENTITY* currentEntity);
void deleteAttribute(FILE* dataDictionary); // Nueva función
void captureAttributes(FILE* dataDictionary, ENTITY currentEntity);
void addAttributesToEntity(FILE* dataDictionary);
void showAttributes(FILE* dataDictionary, ENTITY currentEntity);
void showEntityWithAttributes(FILE* dataDictionary);
void attributeSize(ATTRIBUTE *newAttribute); // Corregido como puntero

int main() {
    FILE* dictionary = initializeDataDictionary("dataDictionary.dat");

    captureEntities(dictionary);
    showEntities(dictionary);
    addAttributesToEntity(dictionary);
    showEntityWithAttributes(dictionary);

    deleteEntity(dictionary); // Prueba eliminar entidad
    deleteAttribute(dictionary); // Prueba eliminar atributo

    fclose(dictionary);
    return 0;
}

// FUNCIONES

FILE* initializeDataDictionary(const char *dictionaryName) {
    long mainHeader = EMPTY_POINTER;

    printf("Initializing Data Dictionary...\n");

    FILE *dictionary = fopen(dictionaryName, "w+b");
    if (!dictionary) {
        perror("Error opening data dictionary");
        exit(EXIT_FAILURE);
    }

    fwrite(&mainHeader, sizeof(mainHeader), 1, dictionary);
    return dictionary;
}

// ENTITIES
int appendEntity(FILE* dataDictionary, ENTITY newEntity) {
    fseek(dataDictionary, 0, SEEK_END);
    long entityDirection = ftell(dataDictionary);

    fwrite(newEntity.name, DATA_BLOCK_SIZE, 1, dataDictionary);
    fwrite(&newEntity.dataPointer, sizeof(long), 1, dataDictionary);
    fwrite(&newEntity.attributesPointer, sizeof(long), 1, dataDictionary);
    fwrite(&newEntity.nextEntity, sizeof(long), 1, dataDictionary);

    return entityDirection;
}

void reorderEntities(FILE* dataDictionary, long currentEntityPointer, const char* newEntityName, long newEntityDirection) {
    long currentEntityDirection = -1;

    fseek(dataDictionary, currentEntityPointer, SEEK_SET);
    fread(&currentEntityDirection, sizeof(currentEntityDirection), 1, dataDictionary);

    if (currentEntityDirection == -1) {
        fseek(dataDictionary, currentEntityPointer, SEEK_SET);
        fwrite(&newEntityDirection, sizeof(long), 1, dataDictionary);
    } else {
        char currentEntityName[DATA_BLOCK_SIZE];
        long nextEntityDirection;
        long nextHeaderPointer;

        fseek(dataDictionary, currentEntityDirection, SEEK_SET);
        fread(currentEntityName, sizeof(currentEntityName), 1, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + (sizeof(long) * 2);

        if (strcmp(currentEntityName, newEntityName) < 0) {
            reorderEntities(dataDictionary, nextHeaderPointer, newEntityName, newEntityDirection);
        } else {
            fseek(dataDictionary, currentEntityPointer, SEEK_SET);
            fwrite(&newEntityDirection, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, newEntityDirection + DATA_BLOCK_SIZE + (sizeof(long) * 2), SEEK_SET);
            fwrite(&currentEntityDirection, sizeof(long), 1, dataDictionary);
        }
    }
}

void createEntity(FILE* dataDictionary) {
    ENTITY newEntity;
    printf("\nEnter the Entity name: ");
    fgets(newEntity.name, sizeof(newEntity.name), stdin);
    newEntity.name[strcspn(newEntity.name, "\n")] = '\0';
    newEntity.dataPointer = EMPTY_POINTER;
    newEntity.attributesPointer = EMPTY_POINTER;
    newEntity.nextEntity = EMPTY_POINTER;

    long entityDirection = appendEntity(dataDictionary, newEntity);
    reorderEntities(dataDictionary, MAIN_ENTITY_POINTER, newEntity.name, entityDirection);
}

ENTITY removeEntity(FILE* dataDictionary, long currentEntityPointer, const char* entityName) {
    long currentEntityDirection = -1;

    fseek(dataDictionary, currentEntityPointer, SEEK_SET);
    fread(&currentEntityDirection, sizeof(currentEntityDirection), 1, dataDictionary);

    if (currentEntityDirection == -1) {
        ENTITY emptyEntity = {0};
        return emptyEntity;
    } else {
        ENTITY resultEntity;
        long nextHeaderPointer;

        fseek(dataDictionary, currentEntityDirection, SEEK_SET);
        fread(resultEntity.name, DATA_BLOCK_SIZE, 1, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + (sizeof(long) * 2);

        if (strcmp(resultEntity.name, entityName) == 0) {
            fread(&resultEntity.dataPointer, sizeof(long), 1, dataDictionary);
            fread(&resultEntity.attributesPointer, sizeof(long), 1, dataDictionary);
            fread(&resultEntity.nextEntity, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, currentEntityPointer, SEEK_SET);
            fwrite(&resultEntity.nextEntity, sizeof(long), 1, dataDictionary);

            return resultEntity;
        } else {
            return removeEntity(dataDictionary, nextHeaderPointer, entityName);
        }
    }
}

void deleteEntity(FILE* dataDictionary) {
    char entityName[DATA_BLOCK_SIZE];
    printf("\nEnter the Entity name to delete: ");
    fgets(entityName, sizeof(entityName), stdin);
    entityName[strcspn(entityName, "\n")] = '\0';

    ENTITY deletedEntity = removeEntity(dataDictionary, MAIN_ENTITY_POINTER, entityName);
    if (deletedEntity.name[0] == '\0') {
        printf("Entity '%s' not found.\n", entityName);
    } else {
        printf("Entity '%s' deleted successfully.\n", deletedEntity.name);
    }
}

// ATRIBUTOS

ATTRIBUTE removeAttribute(FILE* dataDictionary, long currentAttributePointer, const char* attributeName, ENTITY* currentEntity) {
    long currentAttributeDirection = -1;

    fseek(dataDictionary, currentAttributePointer, SEEK_SET);
    fread(&currentAttributeDirection, sizeof(long), 1, dataDictionary);

    if (currentAttributeDirection == -1) {
        ATTRIBUTE emptyAttribute = {0};
        return emptyAttribute;
    } else {
        ATTRIBUTE resultAttribute;
        long nextHeaderPointer;

        fseek(dataDictionary, currentAttributeDirection, SEEK_SET);
        fread(resultAttribute.name, sizeof(resultAttribute.name), 1, dataDictionary);
        nextHeaderPointer = ftell(dataDictionary) + sizeof(bool) + (sizeof(long) * 2);

        if (strcmp(resultAttribute.name, attributeName) == 0) {
            fread(&resultAttribute.isPrimary, sizeof(bool), 1, dataDictionary);
            fread(&resultAttribute.type, sizeof(long), 1, dataDictionary);
            fread(&resultAttribute.size, sizeof(long), 1, dataDictionary);
            fread(&resultAttribute.nextAttribute, sizeof(long), 1, dataDictionary);

            fseek(dataDictionary, currentAttributePointer, SEEK_SET);
            fwrite(&resultAttribute.nextAttribute, sizeof(long), 1, dataDictionary);

            return resultAttribute;
        } else {
            return removeAttribute(dataDictionary, nextHeaderPointer, attributeName, currentEntity);
        }
    }
}

void deleteAttribute(FILE* dataDictionary) {
    char entityName[DATA_BLOCK_SIZE];
    char attributeName[DATA_BLOCK_SIZE];
    ENTITY currentEntity;

    printf("\nEnter the Entity name: ");
    fgets(entityName, sizeof(entityName), stdin);
    entityName[strcspn(entityName, "\n")] = '\0';

    searchEntityByName(dataDictionary, entityName, &currentEntity);
    if (currentEntity.name[0] == '\0') {
        printf("Entity '%s' not found.\n", entityName);
        return;
    }

    printf("\nEnter the Attribute name to delete: ");
    fgets(attributeName, sizeof(attributeName), stdin);
    attributeName[strcspn(attributeName, "\n")] = '\0';

    ATTRIBUTE deletedAttribute = removeAttribute(dataDictionary, currentEntity.attributesPointer, attributeName, &currentEntity);
    if (deletedAttribute.name[0] == '\0') {
        printf("Attribute '%s' not found.\n", attributeName);
    } else {
        printf("Attribute '%s' deleted successfully.\n", deletedAttribute.name);
    }
}
