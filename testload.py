import time
import orjson
from orjson import loads
from orjson import dumps


def main():
    assets = sqlite3.connect('assets.sqlite3')
    assets.row_factory = sqlite3.Row
    state = sqlite3.connect('state.sqlite3')
    state.row_factory = sqlite3.Row

    all_assets = list()
    cur = assets.cursor()

    for table in ("areas", "dgScriptPrototypes","guilds", "itemPrototypes", "npcPrototypes",
                  "rooms", "shops", "zones"):
        data = list()
        start = time.perf_counter()
        print(f"Loading {table}")
        cur.execute(f"SELECT id,data FROM {table}")

        for row in cur.fetchall():
            data.append(loads(row['data']))
        end = time.perf_counter()
        print(f"{table} loaded in {end - start}")
        all_assets.append(data)

    all_state = list()
    cur = state.cursor()

    for table in ("accounts", "characters", "dgScripts", "items", "playerCharacters"):
        data = list()
        start = time.perf_counter()
        print(f"Loading {table}")
        cur.execute(f"SELECT id,data FROM {table}")

        for row in cur.fetchall():
            data.append(loads(row['data']))
        end = time.perf_counter()
        print(f"{table} loaded in {end - start}")
        all_state.append(data)

    return all_state, all_assets


def run_full_test():
    start_time = time.perf_counter()
    all_data = main()
    end_time = time.perf_counter()
    print("Elapsed")
    print(end_time - start_time)

    print("Dumping to disk...")
    start = time.perf_counter()
    with open("test.json", "wb") as outfile:
        outfile.write(dumps(all_data))
        outfile.flush()
    end = time.perf_counter()

    print("Elapsed")
    print(end - start)

def open_dump():
    start_time = time.perf_counter()
    with open("test.json", "rb") as f:
        data = loads(f.read())
    end_time = time.perf_counter()
    print("Elapsed")
    print(end_time - start_time)

if __name__ == '__main__':
    open_dump()

    # now, wait for input...
    input("Press Enter to continue...")