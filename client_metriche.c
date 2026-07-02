#include <stdio.h>
#include <string.h>
#include <math.h>
#include "client_logica.h"

/**
 * @file client_metriche.c
 * @brief Calcoli fisiologici: BMI, BMR, TDEE, kcal obiettivo, macronutrienti.
 */

/* METRICHE FISIOLOGICHE */
float calcola_bmi(float peso_kg, float altezza_cm)
{
    float altezza_m = altezza_cm / 100.0f;
    return peso_kg / (altezza_m * altezza_m);
}

const char *categoria_bmi(float bmi)
{
    if (bmi < 18.5f) return "Sottopeso";
    if (bmi < 25.0f) return "Normopeso";
    if (bmi < 30.0f) return "Sovrappeso";
    return "Obesita'";
}

float calcola_bmr(float peso_kg, float altezza_cm, int eta, char sesso)
{
    float bmr = 10.0f * peso_kg + 6.25f * altezza_cm - 5.0f * (float)eta;
    return (sesso == 'M') ? bmr + 5.0f : bmr - 161.0f;
}

float calcola_tdee(float bmr, float livello_attivita)
{
    return bmr * livello_attivita;
}

float calcola_kcal_obiettivo(float tdee, Obiettivo ob)
{
    switch (ob) {
        case OBIETTIVO_DIMAGRIMENTO: return tdee - 400.0f;
        case OBIETTIVO_MASSA:        return tdee + 250.0f;
        case OBIETTIVO_MANTENIMENTO: return tdee;
        default:                     return tdee;
    }
}

void calcola_macronutrienti(float peso_kg, float kcal_ob, Obiettivo ob,
                            float *proteine, float *grassi, float *carboidrati)
{
    float fattore, kcal_usate;
    fattore      = (ob == OBIETTIVO_MASSA) ? 1.8f : 1.6f;
    *proteine    = peso_kg * fattore;
    *grassi      = (kcal_ob * 0.27f) / 9.0f;
    kcal_usate   = (*proteine) * 4.0f + (*grassi) * 9.0f;
    *carboidrati = (kcal_ob - kcal_usate) / 4.0f;
    if (*carboidrati < 0) *carboidrati = 0;
}

void calcola_metriche(Utente *u)
{
    u->bmr            = calcola_bmr(u->peso_kg, u->altezza_cm, u->eta, u->sesso);
    u->tdee_kcal      = calcola_tdee(u->bmr, u->livello_attivita);
    u->kcal_obiettivo = calcola_kcal_obiettivo(u->tdee_kcal, u->obiettivo);
    calcola_macronutrienti(u->peso_kg, u->kcal_obiettivo, u->obiettivo,
                           &u->proteine_g, &u->grassi_g, &u->carboidrati_g);
}

float stima_settimane(const Utente *u)
{
    float differenza   = u->peso_kg - u->peso_obiettivo_kg;
    float delta_giorno = u->tdee_kcal - u->kcal_obiettivo;
    if (fabsf(differenza) < 0.1f || fabsf(delta_giorno) < 1.0f) return 0.0f;
    return (fabsf(differenza) * 7700.0f) / fabsf(delta_giorno) / 7.0f;
}

float fabbisogno_acqua(float peso_kg, float ore_allenamento)
{
    return peso_kg * 35.0f + ore_allenamento * 500.0f;
}

Obiettivo determina_obiettivo(const Utente *u)
{
    float diff = u->peso_obiettivo_kg - u->peso_kg;
    if (u->bmi > 27.0f || diff < -3.0f) return OBIETTIVO_DIMAGRIMENTO;
    if (u->bmi < 20.0f || diff > 3.0f)  return OBIETTIVO_MASSA;
    return OBIETTIVO_MANTENIMENTO;
}

Livello determina_livello(float livello_attivita)
{
    if (livello_attivita <= 1.375f) return LIVELLO_PRINCIPIANTE;
    if (livello_attivita <= 1.550f) return LIVELLO_INTERMEDIO;
    return LIVELLO_AVANZATO;
}