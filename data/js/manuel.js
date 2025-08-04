// --- Définition des URLs centralisées ---
const apiUrls = {
  getState: (num) => `/sortie-${num}/getState`,
  setState: (num) => `/sortie-${num}/setState`,
};
  // 🔌 Fonction générique pour interagir avec l’ESP
  async function fetchESP(url, data = null) {
    const options = data
      ? {
          method: "POST",
          headers: {
            "content-type": "application/json"
          },
          body: JSON.stringify(data)
        }
      : { method: "GET" };
    try {
      const response = await fetch(url, options);
      if (!response.ok) throw new Error(`Erreur HTTP ${response.status}`);
      const responseData = await response.json();
      return responseData;
    } catch (err) {
      console.error("❌ Erreur fetchESP :", err);
      return null;
    }
  }
// --- Fonctions spécialisées pour les requêtes API ---
const api = {
  async getState(num) {
    const url = apiUrls.getState(num);
    const response = await fetchESP(url);
    if (response?.data?.state !== undefined) {
      return response.data.state;
    }
    console.warn(`⚠️ Réponse inattendue pour l'état de la sortie ${num}`, response);
    return null;
  },

  async setState(num, newState) {
    const url = apiUrls.setState(num);
    const response = await fetchESP(url, { state: newState });
    if (response?.data?.state !== undefined) {
      return response.data.state;
    }
    console.warn(`⚠️ Réponse inattendue pour la commande de la sortie ${num}`, response);
    return null;
  }
};

// --- Reste du code ---
document.addEventListener('DOMContentLoaded', () => {
  const sorties = [1, 2]; // Ajoute d'autres numéros ici si besoin
  sorties.forEach(setupSortie);

  function setupSortie(num) {
    const stateEl = document.getElementById(`sortie-${num}-state`);
    const button = document.getElementById(`sortie-${num}-button`);
    if (!stateEl || !button) {
      console.error(`❌ Éléments HTML manquants pour la sortie ${num}`);
      return;
    }

    async function updateUI() {
      // 💡 On appelle la fonction API pour obtenir l'état
      const state = await api.getState(num);
      if (state !== null) {
        stateEl.textContent = state ? "Active" : "Inactive";
        button.textContent = state ? "Désactiver" : "Activer";
        button.dataset.state = state;
      } else {
        stateEl.textContent = "---";
        button.textContent = "---";
      }
    }

    button.addEventListener('click', async () => {
      button.disabled = true;
      const nouvelEtat = !(button.dataset.state === 'true');
      
      // 💡 On appelle la fonction API pour envoyer la commande
      const sentState = await api.setState(num, nouvelEtat);
      
      // 💡 On met à jour l'UI uniquement si la commande a été envoyée
      if (sentState !== null) {
        await updateUI();
      }
      
      button.disabled = false;
    });

    updateUI(); // Initialisation
  }
});