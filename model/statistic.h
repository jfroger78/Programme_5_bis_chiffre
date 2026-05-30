#ifndef STATISTIC_H
#define STATISTIC_H

// Standard c++ includes
#include <array>
#include <map>

// Qt includes
#include <QString>

struct StatisticsData
{
    // Value
    std::map<int, std::array<int, 24>> datas;
    std::array<std::map<int, int>, 24> datasByColumn;
    
    // Totals
    int total = -1;
    std::map<int, std::array<int, 24>> totalRowColumn;
    std::array<int, 24> totalByColumn = {-1};
    std::map<int, std::array<int, 24>> totalByColumnByValue = {};
};

struct Compo {
    std::map<QString, int> composition = {{"P", 0}, {"3", 0}, {"4", 0}, {"N", 0}};
    std::map<int, int> detailedComposition = {};
    std::map<int, int> detailedWinner = {};
    std::map<int, int> detailAppear = {};
    std::map<int, int> detailByNumber = {};
    std::map<int, int> detailWinnerWithSameValueFromCurrentRace = {};
    std::map<int, int> detailWinnerWithSameValueAppearFromCurrentRace = {};
    std::map<int, int> numberDetail = {};
    int total = 0;
};

struct Composition {
    int number = -1;
    QString column = "";

    Compo keeped;
    Compo removed;

    std::map<int, int> foundedNumber = {};

    void incrementKeepType(const QString& p_type) {
        incrementType(p_type, keeped.composition);
    }

    void incrementRemovedType(const QString& p_type) {
        incrementType(p_type, removed.composition);
    }

    void incrementType(const QString& p_type, std::map<QString, int>& p_mapToIncrement) {
        std::map<QString, int>::iterator it = p_mapToIncrement.find(p_type);
        if(p_mapToIncrement.end() == it) {
            p_mapToIncrement.emplace(std::pair<QString, int>(p_type, 1));
        } else {
            it->second++;
        }
    }
};

struct Ranking {
    int verticalHeaderNumber;

    float firstValue = 0.0;
    float firstTotal = 0.0;
    float firstPercent = 0.0;
    int firstPercentRanking = 0;

    float secondValue = 0.0;
    float secondTotal = 0.0;
    float secondPercent = 0.0;
    int secondPercentRanking = 0;

    float thirdValue = 0.0;
    float thirdTotal = 0.0;
    float thirdPercent = 0.0;
    int thirdPercentRanking = 0;

    const int sumRanking() const {
        return (firstPercentRanking + secondPercentRanking + thirdPercentRanking);
    }
};

struct Rankings {
    std::array<Ranking, 8> datasRemoved;
    std::array<Ranking, 8> datasKeeped;

    void rankFirstPercent(
        const std::array<float, 8>& p_datas,
        std::array<Ranking, 8>& p_datasToRank) {
        for(int tmpIndex = 0; tmpIndex < p_datas.size(); ++tmpIndex) {
            for(int percentIndex = 0; percentIndex < p_datas.size(); ++percentIndex) {
                if(p_datas[tmpIndex] == p_datasToRank[percentIndex].firstPercent) {
                    p_datasToRank[percentIndex].firstPercentRanking = tmpIndex + 1;
                }
            }
        }
    }

    void rankSecondPercent(
        const std::array<float, 8>& p_datas,
        std::array<Ranking, 8>& p_datasToRank) {
        for(int tmpIndex = 0; tmpIndex < p_datas.size(); ++tmpIndex) {
            for(int percentIndex = 0; percentIndex < p_datas.size(); ++percentIndex) {
                if(p_datas[tmpIndex] == p_datasToRank[percentIndex].secondPercent) {
                    p_datasToRank[percentIndex].secondPercentRanking = tmpIndex + 1;
                }
            }
        }
    }

    void rankThirdPercent(
        const std::array<float, 8>& p_datas,
        std::array<Ranking, 8>& p_datasToRank) {
        for(int tmpIndex = 0; tmpIndex < p_datas.size(); ++tmpIndex) {
            for(int percentIndex = 0; percentIndex < p_datas.size(); ++percentIndex) {
                if(p_datas[tmpIndex] == p_datasToRank[percentIndex].thirdPercent) {
                    p_datasToRank[percentIndex].thirdPercentRanking = tmpIndex + 1;
                }
            }
        }
    }
};

#endif // STATISTIC_H