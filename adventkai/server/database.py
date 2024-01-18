from databases import Database
import adventkai
import time
database = Database('sqlite+aiosqlite:///example.db')

BASIC_TABLES = ["zones", "areas", "itemPrototypes", "npcPrototypes", "shops", "guilds",
                "rooms", "dgScriptPrototypes", "accounts", "playerCharacters"]

SCHEMA = [f"CREATE TABLE IF NOT EXISTS {table} (id INTEGER PRIMARY KEY, data TEXT NOT NULL);" for table in BASIC_TABLES]

SCHEMA.append("""
    CREATE TABLE IF NOT EXISTS characters (
           id INTEGER PRIMARY KEY,
           generation INTEGER NOT NULL,
           vnum INTEGER NOT NULL DEFAULT -1,
           name TEXT,
           shortDesc TEXT,
           data TEXT NOT NULL,
           location TEXT NOT NULL DEFAULT '{}',
           relations TEXT NOT NULL DEFAULT '{}'
        );""")

SCHEMA.append("""
    CREATE TABLE IF NOT EXISTS items (
           id INTEGER PRIMARY KEY,
           generation INTEGER NOT NULL,
           vnum INTEGER NOT NULL DEFAULT -1,
           name TEXT,
           shortDesc TEXT,
           data TEXT NOT NULL,
           location TEXT NOT NULL DEFAULT '',
           slot INTEGER NOT NULL DEFAULT 0,
           relations TEXT NOT NULL DEFAULT '{}'
        );""")

SCHEMA.append("""
        

        CREATE TABLE IF NOT EXISTS dgScripts (
        	id INTEGER PRIMARY KEY,
           generation INTEGER NOT NULL,
           vnum INTEGER NOT NULL DEFAULT -1,
           name TEXT,
        	data TEXT NOT NULL,
           location TEXT NOT NULL,
           num INTEGER NOT NULL
        );
""")

SCHEMA.append("""
        CREATE TABLE IF NOT EXISTS globalData (
           name TEXT PRIMARY KEY,
           data TEXT NOT NULL
        );
""")


async def dump_all(data: dict):
    start_time = time.perf_counter()
    async with Database('sqlite+aiosqlite:///example.db') as db:
        async with db.transaction():
            for table in SCHEMA:
                await db.execute(table)

            for table in BASIC_TABLES:
                if found_data := data.pop(table, None):
                    query = f"INSERT INTO {table}(id, data) VALUES(:id, :data)"
                    await db.execute_many(query, found_data)

            if found_data := data.pop("characters", None):
                query = ("INSERT INTO characters (id, generation, vnum, name, shortDesc, data, location, relations)"
                         "VALUES(:id, :generation, :vnum, :name, :shortDesc, :data, :location, :relations)")
                await db.execute_many(query, found_data)

            if found_data := data.pop("items", None):
                query = ("INSERT INTO items (id, generation, vnum, name, shortDesc, data, location, slot, relations)"
                         "VALUES(:id, :generation, :vnum, :name, :shortDesc, :data, :location, :slot, :relations)")
                await db.execute_many(query, found_data)

            if found_data := data.pop("dgscripts", None):
                query = ("INSERT INTO dgscripts (id, generation, vnum, name, data, location, num)"
                         "VALUES(:id, :generation, :vnum, :name, :data, :location, :num)")
                await db.execute_many(query, found_data)

    end_time = time.perf_counter()
    print(f"elapsed dump time: {end_time - start_time}")