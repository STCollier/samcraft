#include <sqlite3.h>
#include "util.h"
#include "../world/chunk.h"

#include "serialize.h"

sqlite3 *db;
char worldFilePath[64];

static int createWorldTable() {
    const char *createTableSQL = "CREATE TABLE IF NOT EXISTS WORLD (worldPosX INT, worldPosY INT, chunk_data BLOB)";
    char *errMsg = 0;

    int rc = sqlite3_exec(db, createTableSQL, 0, 0, &errMsg);

    if (rc != SQLITE_OK) {
        ERROR_MSG("SQL error while creating WORLD table:", errMsg);
        sqlite3_free(errMsg);
        return 1;
    }

    return 0;
}

void initWorldDB(int seed) {
    snprintf(worldFilePath, sizeof(worldFilePath), "res/data/%d.db", seed);
}

void serializeChunk(ivec2 chunkPos, unsigned char *data) {
    struct ChunkData chunkData;
    chunkData.worldPosX = chunkPos[0];
    chunkData.worldPosY = chunkPos[1];
    chunkData.data = calloc(CHUNK_AREA, 1);

    memcpy(chunkData.data, data, CHUNK_AREA);

    if (sqlite3_open(worldFilePath, &db) != SQLITE_OK) {
        ERROR_MSG("Cannot open database:", sqlite3_errmsg(db));
        exit(EXIT_FAILURE);
    }

    if (createWorldTable() != 0) {
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    // Check if a record with the same worldPosX and worldPosY already exists
    sqlite3_stmt *checkStmt;
    const char *checkSQL = "SELECT 1 FROM WORLD WHERE worldPosX = ? AND worldPosY = ?";
    if (sqlite3_prepare_v2(db, checkSQL, -1, &checkStmt, NULL) != SQLITE_OK) {
        ERROR_MSG("SQL error:", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    sqlite3_bind_int(checkStmt, 1, chunkData.worldPosX);
    sqlite3_bind_int(checkStmt, 2, chunkData.worldPosY);

    int result = sqlite3_step(checkStmt);
    sqlite3_finalize(checkStmt);

    if (result == SQLITE_ROW) {
        // A record with the same worldPosX and worldPosY already exists; update it
        const char *updateSQL = "UPDATE WORLD SET chunk_data = ? WHERE worldPosX = ? AND worldPosY = ?";
        sqlite3_stmt *updateStmt;
        if (sqlite3_prepare_v2(db, updateSQL, -1, &updateStmt, NULL) != SQLITE_OK) {
            ERROR_MSG("SQL error:", sqlite3_errmsg(db));
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }

        sqlite3_bind_blob(updateStmt, 1, chunkData.data, CHUNK_AREA, SQLITE_STATIC);
        sqlite3_bind_int(updateStmt, 2, chunkData.worldPosX);
        sqlite3_bind_int(updateStmt, 3, chunkData.worldPosY);

        if (sqlite3_step(updateStmt) != SQLITE_DONE) {
            ERROR_MSG("SQL execution failed:", sqlite3_errmsg(db));
            sqlite3_finalize(updateStmt);
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }

        sqlite3_finalize(updateStmt);
    } else if (result == SQLITE_DONE) {
        // No record with the same worldPosX and worldPosY exists; insert a new record
        const char *insertSQL = "INSERT INTO WORLD (worldPosX, worldPosY, chunk_data) VALUES (?, ?, ?)";
        sqlite3_stmt *insertStmt;
        if (sqlite3_prepare_v2(db, insertSQL, -1, &insertStmt, NULL) != SQLITE_OK) {
            ERROR_MSG("SQL error:", sqlite3_errmsg(db));
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }

        sqlite3_bind_int(insertStmt, 1, chunkData.worldPosX);
        sqlite3_bind_int(insertStmt, 2, chunkData.worldPosY);
        sqlite3_bind_blob(insertStmt, 3, chunkData.data, CHUNK_AREA, SQLITE_STATIC);

        if (sqlite3_step(insertStmt) != SQLITE_DONE) {
            ERROR_MSG("SQL execution failed:", sqlite3_errmsg(db));
            sqlite3_finalize(insertStmt);
            sqlite3_close(db);
            exit(EXIT_FAILURE);
        }

        sqlite3_finalize(insertStmt);
    } else {
        ERROR_MSG("SQL execution failed:", sqlite3_errmsg(db));
        sqlite3_close(db);
        exit(EXIT_FAILURE);
    }

    sqlite3_close(db);
    free(chunkData.data);
}

/*
    // Create a Chunk and allocate memory for data
    struct Chunk chunk;
    chunk.worldPosX = 1;  // Replace with your actual values
    chunk.worldPosY = 2;  // Replace with your actual values
    chunk.data = calloc(512, sizeof(float));
    if (chunk.data == NULL) {
        perror("Memory allocation failed");
        return 1;
    }

    // Initialize some sample data (replace this with your actual data)
    for (int i = 0; i < 512; i++) {
        chunk.data[i] = i * 1.5;
    }

    // Open the SQLite database
    sqlite3 *db;
    if (sqlite3_open("res/world.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    // Create the "WORLD" table if it doesn't exist
    if (createWorldTable(db) != 0) {
        sqlite3_close(db);
        return 1;
    }

    // Check if a record with the same worldPosX and worldPosY already exists
    sqlite3_stmt *checkStmt;
    const char *checkSQL = "SELECT 1 FROM WORLD WHERE worldPosX = ? AND worldPosY = ?";
    if (sqlite3_prepare_v2(db, checkSQL, -1, &checkStmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    sqlite3_bind_int(checkStmt, 1, chunk.worldPosX);
    sqlite3_bind_int(checkStmt, 2, chunk.worldPosY);

    int result = sqlite3_step(checkStmt);
    sqlite3_finalize(checkStmt);

    if (result == SQLITE_ROW) {
        // A record with the same worldPosX and worldPosY already exists; update it
        const char *updateSQL = "UPDATE WORLD SET chunk_data = ? WHERE worldPosX = ? AND worldPosY = ?";
        sqlite3_stmt *updateStmt;
        if (sqlite3_prepare_v2(db, updateSQL, -1, &updateStmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }

        sqlite3_bind_blob(updateStmt, 1, chunk.data, 512 * sizeof(float), SQLITE_STATIC);
        sqlite3_bind_int(updateStmt, 2, chunk.worldPosX);
        sqlite3_bind_int(updateStmt, 3, chunk.worldPosY);

        if (sqlite3_step(updateStmt) != SQLITE_DONE) {
            fprintf(stderr, "SQL execution failed: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(updateStmt);
            sqlite3_close(db);
            return 1;
        }

        sqlite3_finalize(updateStmt);
    } else if (result == SQLITE_DONE) {
        // No record with the same worldPosX and worldPosY exists; insert a new record
        const char *insertSQL = "INSERT INTO WORLD (worldPosX, worldPosY, chunk_data) VALUES (?, ?, ?)";
        sqlite3_stmt *insertStmt;
        if (sqlite3_prepare_v2(db, insertSQL, -1, &insertStmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
            sqlite3_close(db);
            return 1;
        }

        sqlite3_bind_int(insertStmt, 1, chunk.worldPosX);
        sqlite3_bind_int(insertStmt, 2, chunk.worldPosY);
        sqlite3_bind_blob(insertStmt, 3, chunk.data, 512 * sizeof(float), SQLITE_STATIC);

        if (sqlite3_step(insertStmt) != SQLITE_DONE) {
            fprintf(stderr, "SQL execution failed: %s\n", sqlite3_errmsg(db));
            sqlite3_finalize(insertStmt);
            sqlite3_close(db);
            return 1;
        }

        sqlite3_finalize(insertStmt);
    } else {
        fprintf(stderr, "SQL execution failed: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // Close the database connection
    sqlite3_close(db);

    // Free allocated memory
    free(chunk.data);
*/