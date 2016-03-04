# spartan

## Usage
- Clone this resposity to the storage dir of your mysql-server source.
- Compile 
- Copy the `ha_spartan.so` file into the plugin dir of you mysql-server.
- Install plugin `INSTALL PLUGIN Spartan SONAME 'ha_spartan.so'.
- Create table with the option `ENGINE=SPARTAN`
