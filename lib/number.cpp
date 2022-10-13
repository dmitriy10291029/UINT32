#include "number.h"
#include <string>

uint2022_t from_uint(uint32_t value) {
    uint2022_t result;
    if (value >= UINT2022_BASE) {
        result.bits[0] = value % UINT2022_BASE;
        result.bits[1] = value / UINT2022_BASE;
        result.maxDeg = 1;
    } else {
        result.bits[0] = value;
        result.maxDeg = 0;
    }
    initWithZerosFrom(result);
    return result;
}

uint2022_t from_string(const char* buff) {
    if (strlen(buff) > UINT2022_SIZE * 9) {
        showError();
        return uint2022_t();
    }
    uint2022_t result;
    std::string sBuff = buff;
    int buffSize = (int)sBuff.size();
    result.maxDeg = (buffSize - 1) / 9;
    for (char i = 0; i < result.maxDeg; ++i) {
        result.bits[i] = std::stoi(sBuff.substr(buffSize - 9 * (i + 1), 9));
    }
    result.bits[result.maxDeg] = std::stoi(sBuff.substr(0, (buffSize - 1) % 9 + 1));

    initWithZerosFrom(result);
    maxDegSearch(result, result.maxDeg); //if string has zeros at the begining
    return result;
}

uint2022_t operator+(const uint2022_t& lhs, const uint2022_t& rhs) {
    uint2022_t result;
    result.maxDeg = std::max(lhs.maxDeg, rhs.maxDeg);

    int bitSum = 0;

    for (char i = 0; i <= result.maxDeg; ++i) {
        result.bits[i] = bitSum / UINT2022_BASE;
        bitSum = lhs.bits[i] + rhs.bits[i];
        result.bits[i] += bitSum % UINT2022_BASE;
    }

    if (bitSum / UINT2022_BASE == 1) {
        if (result.maxDeg < UINT2022_SIZE - 1) {
            result.bits[++result.maxDeg] = 1;
        } else {
            showError();
            return uint2022_t();
        }
    } 
    initWithZerosFrom(result, result.maxDeg + 1);
    return result;
}

uint2022_t operator-(const uint2022_t& lhs, const uint2022_t& rhs) {
    if (lhs.maxDeg < rhs.maxDeg) {
        showError();
        return uint2022_t();
    }
    uint2022_t result;
    bool minus = 0;
    for (char i = 0; i <= lhs.maxDeg; ++i) {
        if (lhs.bits[i] < rhs.bits[i] + minus) {
            result.bits[i] = UINT2022_BASE + lhs.bits[i] - rhs.bits[i] - minus;
            minus = 1;
        } else {
            result.bits[i] = lhs.bits[i] - rhs.bits[i] - minus;
            minus = 0;
        }
    }
    if (minus == 1) {
        showError();
        return uint2022_t();
    }

    initWithZerosFrom(result, lhs.maxDeg + 1);
    maxDegSearch(result, lhs.maxDeg);
    return result;
}

uint2022_t operator*(const uint2022_t& lhs, const uint32_t& x) {
    //multiplication of uint2022 and uint32
    uint2022_t result; 
    initWithZerosFrom(result, 0);
    for (char i = 0; i <= lhs.maxDeg; ++i) {
        uint60_t product = multiplyTwoSubBase(x, lhs.bits[i]);
        //product has 2 elements: first and second 9 digits of multiplication
        //converting product to bits of result:
        result.bits[i] += product.bits[0];
        if (product.maxDeg == 1) {
            if (i + 1 < UINT2022_SIZE) {
                result.bits[i + 1] += product.bits[1];
            } else {
                showError();
                return uint2022_t();
            }
        }
        
        if (result.bits[i] >= UINT2022_BASE) { //transfer
            if (i + 1 < UINT2022_SIZE) {
                result.bits[i + 1] += result.bits[i] / UINT2022_BASE;
                result.bits[i] %= UINT2022_BASE;
            } else {
                showError();
                return uint2022_t();
            }
        }
    }
    maxDegSearch(result);
    return result;
}

uint2022_t operator*(const uint32_t& x, const uint2022_t& lhs) {
    return lhs * x;
}

uint2022_t operator*(const uint2022_t& lhs, const uint2022_t& rhs) {
    if (lhs.maxDeg + rhs.maxDeg >= UINT2022_SIZE) {
        showError();
        return uint2022_t();
    }
    if (rhs.maxDeg > lhs.maxDeg) {
        return rhs * lhs;
    }
    if (rhs.maxDeg == 0) {
        return lhs * rhs.bits[0];
    }
    uint2022_t result; 
    initWithZerosFrom(result, 0);

    for (char i = 0; i <= rhs.maxDeg; ++i) { //column method
        if (rhs.bits[i] == 0) {
            continue;
        }
        uint2022_t buffer = lhs * rhs.bits[i];
        if (buffer.maxDeg + i >= UINT2022_SIZE) {
            showError();
            return uint2022_t();
        }
        buffer.maxDeg += i;
        for (char j = buffer.maxDeg; j >= 0; --j) {
            buffer.bits[j + i] = buffer.bits[j];
        }
        for (char k = i - 1; k >= 0; --k) {
            buffer.bits[k] = 0;
        }
        result = result + buffer;
    }
    maxDegSearch(result, lhs.maxDeg + rhs.maxDeg + 1);
    return result;
}

uint2022_t operator/(const uint2022_t& lhs, const uint2022_t& rhs) {
    return uint2022_t();
}

bool operator==(const uint2022_t& lhs, const uint2022_t& rhs) {
    if (lhs.maxDeg != rhs.maxDeg) {
        return false;
    }
    for (char i = 0; i <= lhs.maxDeg; ++i) {
        if (lhs.bits[i] != rhs.bits[i]) {
            return false;
        }
    }
    return true;
}

bool operator!=(const uint2022_t& lhs, const uint2022_t& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& stream, const uint2022_t& value) {
    char buffer[10];
    int i = value.maxDeg;
    stream << value.bits[i--];
    for (i; i >= 0; --i) {
        sprintf_s(buffer, "%09d", value.bits[i]);
        stream << buffer;
    }
    return stream;
}

void showError() {
    std::cerr << "Error: undefined behavior" << std::endl;
}

void maxDegSearch(uint2022_t& value, char highDeg) {
    char i = highDeg;
    for (; i > 0 && value.bits[i] == 0; --i) {}
    value.maxDeg = i;
}

void maxDegSearch(uint2022_t& value) {
    char i = UINT2022_SIZE - 1;
    for (; i > 0 && value.bits[i] == 0; --i) {}
    value.maxDeg = i;
}


void initWithZerosFrom(uint2022_t& value, char deg) {
    for (char i = deg; i < UINT2022_SIZE; ++i) {
        value.bits[i] = 0;
    }
}

void initWithZerosFrom(uint2022_t& value) {
    for (char i = value.maxDeg + 1; i < UINT2022_SIZE; ++i) {
        value.bits[i] = 0;
    }
}

uint60_t multiplyTwoSubBase(uint32_t x, uint32_t y) {
    //multiplication of 2 sub 10^9 numbers without using a uint64
    //partitions for 3 parts:
    short bit1[3] = {0, 0, 0};
    bit1[0] = x % 1000;
    bit1[1] = x / 1000 % 1000;
    bit1[2] = x / 1000000;

    short bit2[3] = {0, 0, 0};
    bit2[0] = y % 1000;
    bit2[1] = y / 1000 % 1000;
    bit2[2] = y / 1000000;

    short resShort[6] = {0, 0, 0, 0, 0, 0};

    //column method:
    for (char j = 0; j < 3; ++j) {
        for (char k = 0; k < 3; ++k) {
            uint32_t buff = bit2[j] * bit1[k];
            resShort[j + k] += buff % 1000;
            resShort[j + k + 1] += buff / 1000;
            if (resShort[j + k] >= 1000) {
                resShort[j + k + 1] += resShort[j + k] / 1000;
                resShort[j + k] %= 1000;
            }
        }
    }

    //converting to uint60_t
    uint60_t resUint;
    resUint.bits[0] = 0;
    resUint.bits[1] = 0;

    uint32_t deg = 1;
    for (char j = 0; j < 3; ++j) {
        resUint.bits[0] += resShort[j] * deg;
        deg *= 1000;
    }
    if (resShort[3] + resShort[4] + resShort[5] == 0) {
        resUint.maxDeg = 0;
    } else {
        resUint.maxDeg = 1;
        deg = 1;
        for (char j = 3; j < 6; ++j) {
            resUint.bits[1] += resShort[j] * deg;
            deg *= 1000;
        }
    }
    return resUint;
}