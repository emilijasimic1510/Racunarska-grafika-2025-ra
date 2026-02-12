# 🔍 DEBUG - ZAŠTO NE VIDIŠ BILJKU?

## ✅ **DODAO SAM TEST BILJKU!**

### **OGROMNA biljka na prizemiju:**
```
Pozicija: X=0, Y=prizemlje, Z=4 (CENTAR, NAPRED!)
Veličina: 0.05 (4x VEĆA nego obično!)
```

**Kada pokreneš igru, odmah kada se pojavi prizemlje, trebalo bi da vidiš OGROMNU biljku direktno ispred tebe u centru prostorije!**

---

## 🔍 **PROVERE:**

### **1. Proveri FAJLOVE:**
```
Da li si postavio sva 3 fajla u:
res/models/plants/eb_house_plant_03.obj
res/models/plants/eb_house_plant_03.mtl
res/models/plants/eb_house_plant_03_ren_01.jpg

TAČNA PUTANJA:
C:\...\TvojProjekat\res\models\plants\eb_house_plant_03.obj
C:\...\TvojProjekat\res\models\plants\eb_house_plant_03.mtl
C:\...\TvojProjekat\res\models\plants\eb_house_plant_03_ren_01.jpg
```

### **2. Proveri KONZOLU:**

Kada pokreneš igru, pogledaj crni prozor (konzolu). Vidiš li:
```
ERROR::ASSIMP:: Unable to open file "res/models/plants/eb_house_plant_03.obj"
```

**Ako vidiš ovu grešku:**
- Fajlovi NISU na pravom mestu
- Proveri putanju

**Ako NE vidiš grešku:**
- Fajlovi su OK
- Problem je negde drugde

---

## 🎯 **TEST SCENARIO:**

### **Šta trebalo bi da se desi:**

1. **Pokreneš igru** → Spawn na prizemiju
2. **Odmah ispred tebe** → OGROMNA BILJKA u centru prostorije
3. **Ne možeš je promašiti** → 4x je veća nego obično

### **Ako vidiš biljku:**
✅ **SVE RADI!** Problem je bio samo u poziciji/veličini originalnih biljaka

### **Ako NE vidiš biljku:**
❌ **Fajlovi nisu na pravom mestu** ili greška pri učitavanju

---

## 🛠️ **AKO NE VIDIŠ:**

### **Korak 1: Proveri fajlove**
```
Idi u folder projekta → res → models → plants

Trebalo bi da vidiš:
├── AloePlant.obj
├── potplant2.obj
├── eb_house_plant_03.obj      ← Ovaj?
├── eb_house_plant_03.mtl      ← Ovaj?
└── eb_house_plant_03_ren_01.jpg ← Ovaj?
```

### **Korak 2: Proveri konzolu**
```
Pokreni igru → Odmah pogledaj konzolu

Ima li:
"ERROR::ASSIMP:: Unable to open file..."

DA → Fajlovi nisu tu
NE → Fajlovi su OK, ali nešto drugo ne valja
```

### **Korak 3: Javi mi**
```
1. Vidiš li biljku? (DA/NE)
2. Šta piše u konzoli? (Kopiraj poruku)
3. Da li postoje fajlovi u res/models/plants? (DA/NE)
```

---

## 📊 **DIJAGNOZA:**

| Šta vidiš | Šta znači | Rešenje |
|-----------|-----------|---------|
| **OGROMNA biljka napred** | SVE RADI! | Ukloni test, postavi normalne pozicije |
| **Ništa + greška u konzoli** | Fajlovi nisu tu | Postavi fajlove u `res/models/plants/` |
| **Ništa + nema greške** | Renderovanje problem | Javi mi - debug zajedno |
| **Druge biljke OK, nova ne** | Možda texture problem | Proveri .mtl i .jpg |

---

## 💡 **TRENUTNA POZICIJA TEST BILJKE:**

```cpp
// Linija 769 - TEST BILJKA
if (i == 0) {  // Prizemlje (floor 0)
    glm::mat4 M(1.0f);
    M = glm::translate(M, glm::vec3(0.0f, y, 4.0f));  // Centar, napred
    M = glm::scale(M, glm::vec3(0.05f));  // OGROMNA (4x veća)
    shader.setMat4("uM", M);
    plantHouse.Draw(shader);  // NOVA BILJKA
}
```

**POGLED ODOZGO (prizemlje):**
```
        PREDNJI ZID
             │
   ┌─────────┴──────────┐
   │                    │
   │                    │
   │                    │
   │       🪴          │ ← TEST BILJKA (ogromna!)
   │    (centar)        │
   │                    │
   │       🛗          │ ← Lift
   │                    │
   └────────┬──────────┘
            │
      ZADNJI ZID
```

---

## 🚀 **JAVI MI:**

```
TEST REZULTAT:
[ ] Vidim OGROMNU biljku na prizemiju
[ ] NE vidim biljku
[ ] Konzola pokazuje grešku: _________________
[ ] Fajlovi postoje u res/models/plants/
```

**Onda ćemo znati TAČNO gde je problem!** 🔧
