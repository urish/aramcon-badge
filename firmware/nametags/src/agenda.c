#include <device.h>
#include <display/cfb.h>
#include <string.h>

extern struct device *display;
static s16_t agenda_index = 0;

static const char *agenda[] = {
    // Day 1
    "9:30am Signup & registration",
    "10:30am Opening session | Nir Krakowski",
    "11am What's going on with Aram? | Daniel Goldberg",
    "11:30am Aramcon badge | Uri Shaked",
    "12pm Lunch",
    "1pm [Infra] Public clouds IaaS internals | Liran Alon",
    "1pm [Startups] How to get the most from your investors | Michael Reitblat",
    "2:10pm [Startups] Options 101 | Amit Goldstein",
    "3:20pm [Infra] Autonomous vehicles | Uri Yacoby",
    "3:20pm [Startups] Case study: Guardicore | Pavel Gurvich",
    "4:30pm [Infra] CPU inside | Elad Raz",
    "4:30pm [Startups] Panel: acquisition behind the scenes | Danny Leshem",
    "6pm spaceil | Yariv Bash",
    "7pm Dinner",

    // Day 2
    "8:15am Breakfast",
    "9:10am [Code] Effective code reviews | Yaron Inger",
    "9:10am [Cyber] CTF riddles | Oded Margalit",
    "10:20am [Code] Thoughts about programming | Yaron Dinkin",
    "10:20am [Cyber] amdflaws | Ilia Luk-Zilberman",
    "11:30am [Code] Crazy bugs | Saar Raz",
    "11:30am [Cyber] Stackoveflow, the vulnerability marketplace | Danny Grander",
    "12:30pm Lunch",
    "1:30pm [Crypto] Bitcoin's mining market | Robert Parham",
    "1:30pm [Startups] From biotech to techbio | Omri Amirav Drory",
    "2:40pm [Crypto] Attacking blockchains | Idan Ofrat",
    "2:40pm [Cyber] Attacking neural networks | Yuval Weisglass",
    "3:50pm [Career] Long term investing | Danny Leshem",
    "3:50pm [Cyber] Hardware attacks for firmware dumping | Shachar Menashe",
    "4:50pm Closing session | Nir Krakowski",
};

const u16_t agenda_size = sizeof(agenda) / sizeof(agenda[0]);

static void display_agenda() {
  char line[128];
  strcpy(line, agenda[agenda_index]);
  char *space = strchr(line, ' ');
  space[0] = '\0';
  char *agenda_time = line;
  char *agenda_session = space + 1;
  char *pipe = strchr(agenda_session, '|');
  char *agenda_speaker = NULL;
  if (pipe) {
    pipe[0] = '\0';
    agenda_speaker = pipe + 1;
  }

  cfb_framebuffer_clear(display, true);
  cfb_print(display, agenda_time, 10, 0);
  cfb_print(display, agenda_session, 10, 32);
  if (agenda_speaker) {
    cfb_print(display, agenda_speaker, 10, 72);
  }
  cfb_framebuffer_finalize(display);
}

void agenda_prev() {
  agenda_index--;
  if (agenda_index < 0) {
    agenda_index = agenda_size - 1;
  }
  display_agenda();
}

void agenda_next() {
  agenda_index++;
  if (agenda_index >= agenda_size) {
    agenda_index = 0;
  }
  display_agenda();
}
