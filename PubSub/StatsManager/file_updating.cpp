#include "file_updating.h"


static char* read_file() {
	FILE* file_ptr;
	file_ptr = fopen("stats.txt", "r");
	if (!file_ptr) {
		return NULL;
	}

    // Determine the file size for dynamic allocation
    fseek(file_ptr, 0, SEEK_END);
    long file_size = ftell(file_ptr);
    rewind(file_ptr);  

    char* content = (char*)malloc((file_size + 1));
    if (content == NULL) {
        fclose(file_ptr);
        return NULL;
    }

    size_t size = fread(content, 1, file_size, file_ptr);
    content[size] = '\0';

    fclose(file_ptr); 
    return content;
}

static void overwrite_file(char* stats) {
    FILE* file_ptr = fopen("stats.txt", "w");
    if (file_ptr == NULL) {
        printf("Overwriting file failed.\n");
        return;
    }

    if (fputs(stats, file_ptr) == EOF) {
        printf("Overwriting file failed.\n");
    }

    fclose(file_ptr);  
}

static void parse_file_content(char* file_content, TopicStats file_topic_stats_array[]) {
    char* lines[3];
    char* numbers[5];

    lines[0] = strtok(file_content, "\n");
    lines[1] = strtok(NULL, "\n");
    lines[2] = strtok(NULL, "\n");

    numbers[0] = strtok(lines[0], ",");
    numbers[1] = strtok(NULL, ",");
    numbers[2] = strtok(NULL, ",");
    numbers[3] = strtok(NULL, ",");
    numbers[4] = strtok(NULL, ",");
    (file_topic_stats_array[0]).publish_commands_num = strtoll(numbers[0], NULL, 10);
    (file_topic_stats_array[0]).subscribe_commands_num = strtoll(numbers[1], NULL, 10);
    (file_topic_stats_array[0]).unsubscribe_commands_num = strtoll(numbers[2], NULL, 10);
    (file_topic_stats_array[0]).topic_exists_commands_num = strtoll(numbers[3], NULL, 10);
    (file_topic_stats_array[0]).subscriber_number_commands_num = strtoll(numbers[4], NULL, 10);

    numbers[0] = strtok(lines[1], ",");
    numbers[1] = strtok(NULL, ",");
    numbers[2] = strtok(NULL, ",");
    numbers[3] = strtok(NULL, ",");
    numbers[4] = strtok(NULL, ",");
    (file_topic_stats_array[1]).publish_commands_num = strtoll(numbers[0], NULL, 10);
    (file_topic_stats_array[1]).subscribe_commands_num = strtoll(numbers[1], NULL, 10);
    (file_topic_stats_array[1]).unsubscribe_commands_num = strtoll(numbers[2], NULL, 10);
    (file_topic_stats_array[1]).topic_exists_commands_num = strtoll(numbers[3], NULL, 10);
    (file_topic_stats_array[1]).subscriber_number_commands_num = strtoll(numbers[4], NULL, 10);

    numbers[0] = strtok(lines[2], ",");
    numbers[1] = strtok(NULL, ",");
    numbers[2] = strtok(NULL, ",");
    numbers[3] = strtok(NULL, ",");
    numbers[4] = strtok(NULL, ",");
    (file_topic_stats_array[2]).publish_commands_num = strtoll(numbers[0], NULL, 10);
    (file_topic_stats_array[2]).subscribe_commands_num = strtoll(numbers[1], NULL, 10);
    (file_topic_stats_array[2]).unsubscribe_commands_num = strtoll(numbers[2], NULL, 10);
    (file_topic_stats_array[2]).topic_exists_commands_num = strtoll(numbers[3], NULL, 10);
    (file_topic_stats_array[2]).subscriber_number_commands_num = strtoll(numbers[4], NULL, 10);
}

static bool stringify_stats(TopicStats* topic_stats_array, char* stats, size_t stats_size) {
    size_t offset = 0;

    for (int i = 0; i < 3; ++i) {
        int written = snprintf(stats + offset, stats_size - offset,
            "%llu,%llu,%llu,%llu,%llu\n",
            topic_stats_array[i].publish_commands_num,
            topic_stats_array[i].subscribe_commands_num,
            topic_stats_array[i].unsubscribe_commands_num,
            topic_stats_array[i].topic_exists_commands_num,
            topic_stats_array[i].subscriber_number_commands_num);

        if (written < 0 || (size_t)written >= stats_size - offset) {
            return false;
        }

        offset += written;
    }

    if (offset < stats_size) {
        stats[offset] = '\0';
        return true;
    } else {
        return false;
    }
}

static void show_stats(TopicStats* topic_stats_array) {
    printf("\n***********************************************************\n\n");

    printf("----------------- 'NEWS' TOPIC STATS -----------------\n");
    printf("Number of messages publihsed: %llu\n", (topic_stats_array[0]).publish_commands_num);
    printf("Number of subscriptions: %llu\n", (topic_stats_array[0]).subscribe_commands_num);
    printf("Number of unsubscriptions: %llu\n", (topic_stats_array[0]).unsubscribe_commands_num);
    printf("Number of times clients checked if topic exists: %llu\n", (topic_stats_array[0]).topic_exists_commands_num);
    printf("Number of times clients checked the subscriber number: %llu\n", (topic_stats_array[0]).subscriber_number_commands_num);

    printf("\n----------------- 'UPDATES' TOPIC STATS -----------------\n");
    printf("Number of messages publihsed: %llu\n", (topic_stats_array[1]).publish_commands_num);
    printf("Number of subscriptions: %llu\n", (topic_stats_array[1]).subscribe_commands_num);
    printf("Number of unsubscriptions: %llu\n", (topic_stats_array[1]).unsubscribe_commands_num);
    printf("Number of times clients checked if topic exists: %llu\n", (topic_stats_array[1]).topic_exists_commands_num);
    printf("Number of times clients checked the subscriber number: %llu\n", (topic_stats_array[1]).subscriber_number_commands_num);

    printf("\n----------------- 'WARNINGS' TOPIC STATS -----------------\n");
    printf("Number of messages publihsed: %llu\n", (topic_stats_array[2]).publish_commands_num);
    printf("Number of subscriptions: %llu\n", (topic_stats_array[2]).subscribe_commands_num);
    printf("Number of unsubscriptions: %llu\n", (topic_stats_array[2]).unsubscribe_commands_num);
    printf("Number of times clients checked if topic exists: %llu\n", (topic_stats_array[2]).topic_exists_commands_num);
    printf("Number of times clients checked the subscriber number: %llu\n", (topic_stats_array[2]).subscriber_number_commands_num);

    printf("\n***********************************************************\n\n");
}


static void reset_stats(TopicStats* topic_stats_array) {
    for (int i = 0; i < 3; i++) {
        (topic_stats_array[i]).publish_commands_num = 0;
        (topic_stats_array[i]).subscribe_commands_num = 0;
        (topic_stats_array[i]).unsubscribe_commands_num = 0;
        (topic_stats_array[i]).topic_exists_commands_num = 0;
        (topic_stats_array[i]).subscriber_number_commands_num = 0;
    }
}


void read_and_show_stats() {
    char* file_content = read_file();
    if (!file_content) {
        printf("Reading from file failed.\n");
        return;
    }

    TopicStats file_topic_stats_array[3];

    if (!strcmp(file_content, "")) {
        reset_stats(file_topic_stats_array);
        show_stats(file_topic_stats_array);
    } else {
        parse_file_content(file_content, file_topic_stats_array);
        show_stats(file_topic_stats_array);
    }
}


void update_and_show_stats(TopicStats* topic_stats_array) {
    char* file_content = read_file();
    if (!file_content) {
        printf("Reading from file failed.\n");
        return;
    }

    char stats[400];

    if (!strcmp(file_content, "")) {
        bool success = stringify_stats(topic_stats_array, stats, 400);
        if (success) {
            overwrite_file(stats);
        } else {
            printf("Stringifying stats failed.\n");
        }

        show_stats(topic_stats_array);
    } else {
        TopicStats file_topic_stats_array[3];
        parse_file_content(file_content, file_topic_stats_array);

        for (int i = 0; i < 3; i++) {
            (file_topic_stats_array[i]).publish_commands_num += (topic_stats_array[i]).publish_commands_num;
            (file_topic_stats_array[i]).subscribe_commands_num += (topic_stats_array[i]).subscribe_commands_num;
            (file_topic_stats_array[i]).unsubscribe_commands_num += (topic_stats_array[i]).unsubscribe_commands_num;
            (file_topic_stats_array[i]).topic_exists_commands_num += (topic_stats_array[i]).topic_exists_commands_num;
            (file_topic_stats_array[i]).subscriber_number_commands_num += (topic_stats_array[i]).subscriber_number_commands_num;
        }

        show_stats(file_topic_stats_array);

        bool success = stringify_stats(file_topic_stats_array, stats, 400);
        if (success) {
            overwrite_file(stats);
        } else {
            printf("Stringifying stats failed.\n");
        }
    }

    reset_stats(topic_stats_array);
}




