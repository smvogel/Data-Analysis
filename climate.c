#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_STATES 50
#define NUM_TOKENS 9

struct climate_info
{
    char code[3];
    unsigned long num_records;
    long double sum_temperature;
    long double humidity;
    double snow;
    long double cloud_cover;
    double lightning_strikes;
    long double pressure;
    long double surface_temperature;
    double max_temperature;
    double min_temperature;
    long int max_temperature_date;
    long int min_temperature_date;
};

void analyze_file(FILE *file, struct climate_info *states[], int num_states);
void print_report(struct climate_info *states[], int num_states);

double kelvin_to_fahrenheit(long double kelvin); // Helper function for temperature conversion

int main(int argc, char *argv[])
{


    if (argc < 2)
    {
        printf("Usage: %s tdv_file1 tdv_file2 ... tdv_fileN \n", argv[0]);
        return EXIT_FAILURE;
    }


    struct climate_info *states[NUM_STATES] = {NULL};

    int i;
    for (i = 1; i < argc; ++i)
    {

        FILE *file = fopen(argv[i], "r");
        printf("Opening file: %s\n", argv[i]);



        if (file == NULL)
        {
            printf("Error opening file: %s\n", argv[i]);
            continue;
        }

        analyze_file(file, states, NUM_STATES);
        fclose(file);
    }

    print_report(states, NUM_STATES);

    return 0;
}

void analyze_file(FILE *file, struct climate_info **states, int num_states)
{
    const int line_sz = 100;
    char line[line_sz];
    while (fgets(line, line_sz, file) != NULL)
    {

        char *tokenArray[NUM_TOKENS];//store tokens
        int idx = 0;
        char *token = strtok(line, "\t");//tokenize line

        while (token != NULL && idx < NUM_TOKENS)
        {   //store tokens in tokenArray
            tokenArray[idx++] = token;
            token = strtok(NULL, "\t");
        }

        if (idx != NUM_TOKENS)
        {   //check if correct number of tokens
            printf("Error: Incorrect number of tokens in line: %s\n", line);
            continue;
        }
        /* parse tokens. Convert to data types */
        char *state_code = tokenArray[0];
        long int timestamp_ms = atol(tokenArray[1]);
        time_t timestamp = (time_t)(timestamp_ms / 1000); // convert to seconds
        long double humidity = strtold(tokenArray[3], NULL);
        double snow = atof(tokenArray[4]);
        long double cloud_cover = strtold(tokenArray[5], NULL);
        unsigned long lightning_strikes = (unsigned long)atoi(tokenArray[6]);
        long double pressure = strtold(tokenArray[7], NULL);
        long double surface_temperature = strtold(tokenArray[8], NULL);

        // Find climate_info for state
        int j;
        struct climate_info *info = NULL;
        for (j = 0; j < num_states; ++j)
        {
            if (states[j] != NULL && strcmp(states[j]->code, state_code) == 0)
            {
                info = states[j];
                break;
            }
        }

        if (info != NULL)
        {
            info->num_records++;
            info->sum_temperature += surface_temperature;
            info->humidity += humidity;
            if (snow > 0.0)
            {
                info->snow++;
            }
            info->cloud_cover += cloud_cover;
            info->lightning_strikes += lightning_strikes;
            info->pressure += pressure;

            if (surface_temperature > info->max_temperature)
            {
                info->max_temperature = surface_temperature;
                info->max_temperature_date = timestamp;
            }
            if (surface_temperature < info->min_temperature)
            {
                info->min_temperature = surface_temperature;
                info->min_temperature_date = timestamp;
            }
        }
        else
        {
            info = malloc(sizeof(struct climate_info));
            if (info == NULL)
            {
                printf("Error: Unable to allocate memory for climate_info\n");
                continue;
            }

            strncpy(info->code, state_code, sizeof(info->code) - 1);
            info->code[sizeof(info->code) - 1] = '\0';
            info->num_records = 1;
            info->sum_temperature = surface_temperature;
            info->humidity = humidity;
            info->snow = (snow > 0.0) ? 1 : 0;
            info->cloud_cover = cloud_cover;
            info->lightning_strikes = (lightning_strikes > 0) ? 1 : 0;
            info->pressure = pressure;
            info->max_temperature = surface_temperature;
            info->min_temperature = surface_temperature;
            info->max_temperature_date = timestamp;
            info->min_temperature_date = timestamp;

            for (j = 0; j < num_states; ++j) // add to states array
            {
                if (states[j] == NULL)
                {
                    states[j] = info;
                    break;
                }
            }

            if (j == num_states)
            {
                printf("Error: No space left in states array\n");
                free(info);
            }
        }
    } // end while loop
}

double kelvin_to_fahrenheit(long double kelvin)
{
    return (kelvin - 273.15) * 9.0 / 5.0 + 32.0;
}

void print_report(struct climate_info *states[], int num_states)
{
    printf("States found:\n");
    int i;
    for (i = 0; i < num_states; ++i)
    {
        if (states[i] != NULL)
        {
            struct climate_info *info = states[i];
            printf("%s ", info->code);
        }
    }
    printf("\n");

    for (i = 0; i < num_states; ++i)
    {
        if(states[i] != NULL)
        {
            struct climate_info *info = states[i];
            printf("-- State: %s --\n", info->code);
            printf("Number of Records: %lu\n", info->num_records);
            printf("Average Humidity: %.1Lf%%\n", info->humidity / info->num_records);
            printf("Average Temperature: %.1fF\n", kelvin_to_fahrenheit(info->sum_temperature / info->num_records));
            printf("Max Temperature: %.1fF\n", kelvin_to_fahrenheit(info->max_temperature));

            char max_temp_time_str[26];
            ctime_r(&info->max_temperature_date, max_temp_time_str);
            max_temp_time_str[strcspn(max_temp_time_str, "\n")] = 0;
            printf("Max Temperature on: %s\n", max_temp_time_str);

            printf("Min Temperature: %.1fF\n", kelvin_to_fahrenheit(info->min_temperature));

            char min_temp_time_str[26];
            ctime_r(&info->min_temperature_date, min_temp_time_str);
            min_temp_time_str[strcspn(min_temp_time_str, "\n")] = 0;
            printf("Min Temperature on: %s\n", min_temp_time_str);

            printf("Lightning Strikes: %.0f\n", info->lightning_strikes);
            printf("Records with Snow Cover: %.0f\n", info->snow);
            printf("Average Cloud Cover: %.1Lf%%\n", info->cloud_cover / info->num_records);
        }
    }
}
