# 🌿 **SVE BILJKE - SREDNJA VELIČINA!**

## ✅ **ŠTA SAM URADIO:**

### **Promene:**
- ❌ Uklonjena OGROMNA test biljka
- ✅ **SVE biljke sada 0.020** (srednja veličina)
- ✅ **SVE iste veličine** (bilo je 0.012, sada 0.020)
- ✅ **67% veće** nego pre

---

## 📊 **VELIČINE:**

| Biljka | Stara veličina | Nova veličina | Promena |
|--------|----------------|---------------|---------|
| Aloe | 0.012 | **0.020** | +67% |
| House Plant | 0.012 | **0.020** | +67% |
| Pot Plant | 0.012 | **0.020** | +67% |

**Sve su ISTE veličine!** 🌿🌿🌿

---

## 🎯 **KOD (Linije 766-793):**

```cpp
// ========== BILJKE - SVE ISTE VELIČINE (srednje) ==========

if (i % 3 == 0) {
    M = glm::scale(M, glm::vec3(0.020f));  // ← SREDNJA VELIČINA (sve iste!)
    plantAloe.Draw(shader);
}

if (i % 3 == 1) {
    M = glm::scale(M, glm::vec3(0.020f));  // ← SREDNJA VELIČINA (sve iste!)
    plantHouse.Draw(shader);
}

if (i % 3 == 2) {
    M = glm::scale(M, glm::vec3(0.020f));  // ← SREDNJA VELIČINA (sve iste!)
    plantAloe.Draw(shader);
}

if (i % 3 == 0 && i != -1) {
    M = glm::scale(M, glm::vec3(0.020f));  // ← SREDNJA VELIČINA (sve iste!)
    plantPot.Draw(shader);
}
```

---

## 🔧 **AKO ŽELIŠ DA PROMENIŠ:**

### **Veće biljke:**
```cpp
M = glm::scale(M, glm::vec3(0.025f));  // 25% veće
M = glm::scale(M, glm::vec3(0.030f));  // 50% veće
```

### **Manje biljke:**
```cpp
M = glm::scale(M, glm::vec3(0.015f));  // 25% manje
M = glm::scale(M, glm::vec3(0.012f));  // Originalna veličina
```

### **Različite veličine (primer):**
```cpp
// Aloe - velika
M = glm::scale(M, glm::vec3(0.025f));

// House Plant - srednja
M = glm::scale(M, glm::vec3(0.020f));

// Pot Plant - mala
M = glm::scale(M, glm::vec3(0.015f));
```

---

## 📍 **TRENUTNI RASPORED:**

### **8 spratova, 19 biljaka:**

| Sprat | Levo napred | Desno napred | Levo nazad | Desno nazad |
|-------|-------------|--------------|------------|-------------|
| **-1** | Aloe (0.020) | House (0.020) | - | Pot (0.020) |
| **0** | Aloe (0.020) | - | Aloe (0.020) | Pot (0.020) |
| **1** | - | House (0.020) | - | - |
| **2** | Aloe (0.020) | - | Aloe (0.020) | - |
| **3** | Aloe (0.020) | House (0.020) | - | Pot (0.020) |
| **4** | - | House (0.020) | - | - |
| **5** | Aloe (0.020) | - | Aloe (0.020) | - |
| **6** | Aloe (0.020) | House (0.020) | - | Pot (0.020) |

**SVE biljke: 0.020 veličina!** ✅

---

## 🎨 **VIZUELNI REZULTAT:**

```
Stare biljke:  🌱 (0.012 - male)
Nove biljke:   🌿 (0.020 - srednje)

Razlika: 67% veće, ali ne prevelike!
```

---

## ✅ **FINALNO:**

- [x] Test biljka uklonjena
- [x] SVE biljke 0.020 (srednje)
- [x] SVE iste veličine
- [x] Dobra proporcija
- [x] Lako vidljive

**Kompajliraj i vidi rezultat!** 🌿✨
