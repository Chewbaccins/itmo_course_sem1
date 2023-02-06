#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <random>
#include <sstream>
#include <Windows.h>

const int MAX_TURNS = 10;

bool get_random_bool(double probability) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::bernoulli_distribution distribution(probability);
  return distribution(gen);
}

int get_random_int(int limit_low, int limit_high) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> distribution(limit_low, limit_high - 1);
  return distribution(gen);
}

double get_random_double(double limit) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution distribution(0.0, limit);
  return distribution(gen);
}

class City
{
private:
    int  population_, space_;
    float wheat_;
    bool can_run_round_, plague_;
    int population_died_, population_arrived_;
    int wheat_risen_, wheat_per_akr_, wheat_stollen_, price_;
    int sum_years_population_, sum_dead_;
public:
    int round_; 
    // конструктор для новой игры
    City(): round_(0), population_(100), wheat_(2800), space_(1000),
            can_run_round_(true), plague_(false),
            population_died_(0), population_arrived_(0),
            wheat_risen_(0), wheat_per_akr_(0), wheat_stollen_(0), price_(get_random_int(17, 26)),
            sum_years_population_(0), sum_dead_(0) {}
    // конструктор для загруженной игры
    City(int round, int population, int wheat, int space,
        bool can_run_round, bool plague,
        int population_died, int population_arrived,
        int wheat_risen, int wheat_per_akr, int wheat_stollen, int price,
        int sum_years_population, int sum_dead): 
            round_(round), population_(population), wheat_(wheat), space_(space),
            can_run_round_(can_run_round), plague_(plague),
            population_died_(population_died), population_arrived_(population_arrived),
            wheat_risen_(wheat_risen), wheat_per_akr_(wheat_per_akr), wheat_stollen_(wheat_stollen), price_(price),
            sum_years_population_(sum_years_population), sum_dead_(sum_dead) {}
    // Можем продолжать игру?
    bool can_run_round() {return can_run_round_;}
    int round_inc() {round_++; return round_;}
    float get_P() {return 1.0*sum_dead_/sum_years_population_;}
    int get_L() {return space_/population_;}
    // Проверка возможности хода, по введенным игроком данных
    bool check_input(int acre_diff, int wheat_eat, int acre_to_sow);
    // Совершить ход игры, вернулось true - значит все хорошо, вернулось false - значит игрок проиграл
    bool make_turn(int acre_diff, int wheat_eat, int acre_to_sow);
    // Вывод инфы
    void print_current_city_state();
    std::string read_all_param();
    void print_state_short();
    // Деструктор
    ~City() {};
};

void City::print_state_short() {
    std::cout << "\nBe merciful! We dont have enough resources! we got only " << population_ << " people, " << wheat_ << " bushels and " << space_ << " acres!\n";
}

void City::print_current_city_state() {
    std::cout << "\nIn a year " << round_ << " of your ruling\n";
    if (population_died_ > 0) {
        std::cout << population_died_ << " people died of hunger";
        if (population_arrived_ <= 0) std::cout << ";\n";
    }
    if (population_arrived_ > 0) {
        if (population_died_ > 0) std::cout << ", and ";
        std::cout << population_arrived_ << " people arrived in your city;\n";
    }
    if (plague_) std::cout << "Plague took half of your people;\n";
    std::cout << "City population is currently " << population_ << " people;\n";
    std::cout << "We collected " << wheat_risen_;
    std::cout << " bushels, at " << wheat_per_akr_ << " bushel per acre;\n";
    std::cout << "Rats took " << wheat_stollen_ << " bushels, we have " << wheat_ << " bushel left at barns;\n";
    std::cout << "City size is " << space_ << " acre;\n";
    std::cout << "1 acre is now at price of " << price_ << " bushels;\n\n";
}

std::string City::read_all_param() {
    std::stringstream result_string;
    result_string << round_ << std::endl << population_ << std::endl << wheat_ << std::endl << space_ << std::endl << can_run_round_ << std::endl << plague_ << std::endl << population_died_ << std::endl << population_arrived_ << std::endl << wheat_risen_ << std::endl << wheat_per_akr_ << std::endl << wheat_stollen_ << std::endl << price_ << std::endl << sum_years_population_ << std::endl << sum_dead_ << std::endl;
    return result_string.str();
}

// проверяем наличие файла сохранения
bool save_file_exists() {
    std::string name = "SaveFile";
    struct stat buffer;   
    return (stat (name.c_str(), &buffer) == 0);
}

void load_game(City &city) {
    if(save_file_exists()) {
        std::cout << "\nsave file found, loading...\n";
        int round, population, space;
        float wheat;
        int can_run_round_int, plague_int;
        int population_died, population_arrived;
        // сколько собрали бушелей с акра, сколько украли мыши, цена за акр
        int wheat_risen, wheat_per_akr, wheat_stollen, price;
        int sum_years_population, sum_dead;

        std::ifstream fin("SaveFile");
        fin >> round >> population >> wheat >> space >> can_run_round_int >> plague_int >> population_died >> population_arrived >> wheat_risen >> wheat_per_akr >> wheat_stollen >> price >> sum_years_population >> sum_dead;

        bool can_run_round = true, plague = true;
        if (can_run_round_int == 0) can_run_round = false;
        if (plague_int == 0) plague = false;
        city = City(round, population, wheat, space,
                    can_run_round, plague,
                    population_died, population_arrived,
                    wheat_risen, wheat_per_akr, wheat_stollen, price,
                    sum_years_population, sum_dead);

        fin.close();
    }
}

void save_progress(City city) {
    std::ofstream fout("SaveFile");
    fout << city.read_all_param();
    fout.close();
}

bool City::check_input(int acre_diff, int wheat_eat, int acre_to_sow) {
    if (space_ + acre_diff < 0) {
        print_state_short();
        return false; // если ушел в минус по земле
    }
    if (acre_to_sow > space_ + acre_diff) acre_to_sow = space_ + acre_diff; //если приказали засеять больше чем возможно
    if (wheat_ - (wheat_eat + acre_diff * price_ + (acre_to_sow / 2)) < 0) {
        print_state_short();
        return false; // если тратишь больше пшеницы чем доступно
    }
    return true;
}

bool handle_input(int &acre_diff, int &wheat_eat, int &acre_to_sow, City &city) {
    int acre_buy = 0;
    int acre_sell = 0;
    std::string temp_string = "";
    std::cout << "What is your next move Lord? (save and leave? y/n)\n";
    std::cin >> temp_string;
    if (temp_string == "y") {
        save_progress(city);
        return false;
    }
    do {
        std::cout << "How many acre are you willing to buy? ";
        std::cin >> acre_buy;
        std::cout << "How many acre are you willing to sell? ";
        std::cin >> acre_sell;
        std::cout << "How many bushel are you willing to eat? ";
        std::cin >> wheat_eat;
        std::cout << "How many acre are you willing to seed? ";
        std::cin >> acre_to_sow;
        acre_diff = acre_buy - acre_sell;
    } while (!city.check_input(acre_diff, wheat_eat, acre_to_sow));
    return true;
}

bool City::make_turn(int acre_diff, int wheat_eat, int acre_to_sow) {
    // совершаем действия пользователя после покупки-продажи земли
    space_ = space_ + acre_diff;
    if (acre_to_sow > space_) acre_to_sow = space_; //если приказали засеять больше чем возможно
    wheat_ = wheat_ - (acre_diff * price_ + (acre_to_sow / 2));

    // совершаем внутренние игровые действия
    population_died_ = population_ - wheat_eat / 20; // сколько погибнет от голода
    if (population_died_ < 0) population_died_ = 0;
    if ((1.0 * population_died_ / population_) > 0.45) {
        std::cout << "\nIT'S YOUR FAULT THAT WE DIED OF HUNGER!\n\n"; // более 45% померло от голода
        return false;
    }
    population_ = population_ - population_died_; // осталось обрабатывать поля
    wheat_ = wheat_ - wheat_eat;

    wheat_per_akr_ = get_random_int(1, 6);
    wheat_risen_ = wheat_per_akr_ * ((10 * population_ > acre_to_sow) ? acre_to_sow : (10 * population_)); // получено с обработанных полей
    wheat_ += wheat_risen_;
    wheat_stollen_ = (int)(get_random_double(0.07) * wheat_);
    wheat_ = wheat_ - wheat_stollen_; //украли крысы

    population_arrived_ = (int)(population_died_ / 2 + (int)(1.0 * (5 - wheat_per_akr_) * wheat_ / 600) + 1);
    if (population_arrived_ < 0) population_arrived_ = 0;
    if (population_arrived_ > 50) population_arrived_ = 50;
    population_ += population_arrived_;

    sum_years_population_ += population_;
    plague_ = get_random_bool(0.15);
    if (plague_) population_ = (int)population_ / 2;

    price_ = get_random_int(17, 26);
    sum_years_population_ += 
    sum_dead_ += population_died_;
    return true;
}

void start_game(City &city) {
    int acre_diff = 0;
    int wheat_eat = 0;
    int acre_to_sow = 0;
    int round = city.round_;
    while(city.can_run_round()) {
        city.print_current_city_state();
        if(!handle_input(acre_diff, wheat_eat, acre_to_sow, city)) {
            return;
        };
        if (city.make_turn(acre_diff, wheat_eat, acre_to_sow)){
            round = city.round_inc();
        } else {
            break;
        }
        if (round == 10) {
            float P = city.get_P();
            int L = city.get_L();
            if (P > 0.33 && L < 7) std::cout << "\nBecause of your incompetence in government, the people staged a riot and expelled you from their cities. Now you are forced to drag out a miserable existence in exile.\n\n";
            else if (P > 0.10 && L < 9) std::cout << "\nYou ruled with an iron fist, like Nero and Ivan the Terrible. The people breathed a sigh of relief, and no one else wants to see you as a ruler\n\n";
            else if (P > 0.03 && L < 10) std::cout << "\nYou did quite well, you certainly have detractors, but many would like to see you at the head of the city again\n\n";
            else std::cout << "\nFantastic! Charlemagne, Disraeli and Jefferson couldn't have done better together\n\n";
            break;
        }
    }
}

int main(int argc, char const *argv[]) {
    City Hammurabi;
    load_game(Hammurabi);
    start_game(Hammurabi);
    return 0;
}
