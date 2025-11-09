//COPIED FROM "SciddicaTCell.h"
#include "../hybridArray/HybridArrayMarkers.h" //<-- for "FIELD" macro

#include <cstdio>     // sprintf
#include <cstdlib>    // atof
#include <cstring>    // strlen
#include <algorithm>  // std::fill_n, std::max, std::copy_n
#include <string>


class SciddicaTCell
{
public:
    static constexpr int NUMBER_OF_OUTFLOWS = 4;

    static constexpr int minH = 0;
    static constexpr int maxH = 5;
    static constexpr int minZ = 0;
    static constexpr int maxZ = 1300;

    FIELD double z{}; /// z is altitude in meters
    FIELD double h{}; /// h is quantity in debris in meters
    FIELD double f[NUMBER_OF_OUTFLOWS]{};

	explicit SciddicaTCell() = default;

    SciddicaTCell(double z, double h, double f[NUMBER_OF_OUTFLOWS])
        : z(z), h(h)
    {
        std::copy_n(f, NUMBER_OF_OUTFLOWS, this->f);
    }

    void startStep(int /*step*/)
    {
    }

    double getZ() const
    {
        return z;
    }

    void setZ(double z)
    {
        this->z = z;
    }

    double getH() const
    {
        return h;
    }

    void setH(double h)
    {
        this->h = h;
    }

    double getF(int i) const
    {
        return f[i];
    }

    void setF(double value, int i)
    {
        this->f[i] = value;
    }

    /** @brief Parse a cell encoded as "[<z>,<h>]" into z and h values.
     * Expected input format:
     *   [<double>,<double>]
     * Examples:
     *   "[530.000000,231.000000]"
     *   "[12.340000,56.780000]"
     * The function parses both doubles directly from the raw C string,
     * without creating any temporary std::string (for performance).
     * On success:
     *   - assigns parsed values to z and h
     *   - updates maxH if h is greater
     * On error (missing comma or brackets):
     *   - sets z = 0, h = 0
     *   - does NOT modify maxH */
    void composeElement(char* str) 
    {
        if (! str)
        {
            z = h = 0.0;
            return;
        }

        const char valuesSeparator = ',';
        char* pComma = static_cast<char*>(memchr(str, valuesSeparator, strlen(str)));
        if (! pComma) // Invalid format — no comma
        {
            z = h = 0.0;
            return;
        }

        // Terminate first number, so atof reads cleanly
        *pComma = '\0';

        // Parse values
        z = atof(str + 1);
        h = atof(pComma + 1);
    }

    std::string stringEncoding(const char* str=NULL) const 
    {
        char zstr[512];
        if (NULL == str)
        {
            sprintf(zstr, "[%0.6f,%0.6f]", z, h);
            return std::string(zstr);
        }
        if (strcmp(str,"h") == 0)
            sprintf(zstr, "%0.6f", h); // LAVA						
        if (strcmp(str,"z") == 0)
            sprintf(zstr, "%0.6f", z); // TOPOLOGIA
        
        return std::string(zstr);
    }

    /*
    Color outputValue(const char* str) const override
    {
        if (str)
        {
            if (strcmp(str, "h") == 0)
                return Color((h-minH)*255/(maxH-minH), 0, 0); // LAVA
            if (strcmp(str, "z") == 0)
                return Color((z-minZ)*255/(maxZ-minZ), 0, 0); // TOPOLOGIA
        }

        if (h > 0)
            return Color(255, 0, 0);
        else
            return Color(255, 255, 255);
    }
    */
};