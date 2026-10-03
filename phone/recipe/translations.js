// UI strings + built-in starter recipes for the recipe phone plugin. Locale
// follows the system language (navigator.language) — French is the default.
// Supported: fr, en, es, it, de, zh-CN.

const RECIPES = {
  fr: [
    {
      name: 'Crêpes',
      steps: [
        { text: 'Mélanger la farine, les œufs, le lait et le sucre.', minutes: 2 },
        { text: 'Laisser reposer la pâte.', minutes: 10 },
        { text: 'Chauffer la poêle et verser une louche de pâte.', minutes: 1 },
        { text: 'Faire cuire chaque crêpe jusqu’à ce qu’elle soit dorée.', minutes: 3 },
      ],
    },
    {
      name: 'Pâtes à la tomate',
      steps: [
        { text: 'Porter une grande casserole d’eau salée à ébullition.', minutes: 5 },
        { text: 'Cuire les pâtes al dente.', minutes: 9 },
        { text: 'Faire revenir l’ail et la tomate dans l’huile.', minutes: 4 },
        { text: 'Égoutter, mélanger la sauce et servir.', minutes: 1 },
      ],
    },
  ],
  en: [
    {
      name: 'Pancakes',
      steps: [
        { text: 'Whisk the flour, eggs, milk and sugar together.', minutes: 2 },
        { text: 'Let the batter rest.', minutes: 10 },
        { text: 'Heat the pan and pour in a ladle of batter.', minutes: 1 },
        { text: 'Cook each pancake until golden.', minutes: 3 },
      ],
    },
    {
      name: 'Tomato pasta',
      steps: [
        { text: 'Bring a large pot of salted water to a boil.', minutes: 5 },
        { text: 'Cook the pasta al dente.', minutes: 9 },
        { text: 'Sauté the garlic and tomato in oil.', minutes: 4 },
        { text: 'Drain, toss with the sauce and serve.', minutes: 1 },
      ],
    },
  ],
  es: [
    {
      name: 'Tortitas',
      steps: [
        { text: 'Batir la harina, los huevos, la leche y el azúcar.', minutes: 2 },
        { text: 'Dejar reposar la masa.', minutes: 10 },
        { text: 'Calentar la sartén y verter un cazo de masa.', minutes: 1 },
        { text: 'Cocinar cada tortita hasta que esté dorada.', minutes: 3 },
      ],
    },
    {
      name: 'Pasta al tomate',
      steps: [
        { text: 'Llevar una olla grande de agua con sal a ebullición.', minutes: 5 },
        { text: 'Cocer la pasta al dente.', minutes: 9 },
        { text: 'Sofreír el ajo y el tomate en aceite.', minutes: 4 },
        { text: 'Escurrir, mezclar con la salsa y servir.', minutes: 1 },
      ],
    },
  ],
  it: [
    {
      name: 'Pancake',
      steps: [
        { text: 'Mescolare farina, uova, latte e zucchero.', minutes: 2 },
        { text: 'Lasciar riposare la pastella.', minutes: 10 },
        { text: 'Scaldare la padella e versare un mestolo di pastella.', minutes: 1 },
        { text: 'Cuocere ogni pancake finché non è dorato.', minutes: 3 },
      ],
    },
    {
      name: 'Pasta al pomodoro',
      steps: [
        { text: 'Portare a bollore una pentola d’acqua salata.', minutes: 5 },
        { text: 'Cuocere la pasta al dente.', minutes: 9 },
        { text: 'Soffriggere aglio e pomodoro nell’olio.', minutes: 4 },
        { text: 'Scolare, condire con la salsa e servire.', minutes: 1 },
      ],
    },
  ],
  de: [
    {
      name: 'Pfannkuchen',
      steps: [
        { text: 'Mehl, Eier, Milch und Zucker verrühren.', minutes: 2 },
        { text: 'Den Teig ruhen lassen.', minutes: 10 },
        { text: 'Die Pfanne erhitzen und eine Kelle Teig hineingeben.', minutes: 1 },
        { text: 'Jeden Pfannkuchen goldbraun backen.', minutes: 3 },
      ],
    },
    {
      name: 'Tomatenpasta',
      steps: [
        { text: 'Einen großen Topf Salzwasser zum Kochen bringen.', minutes: 5 },
        { text: 'Die Pasta al dente kochen.', minutes: 9 },
        { text: 'Knoblauch und Tomate in Öl anbraten.', minutes: 4 },
        { text: 'Abgießen, mit der Soße mischen und servieren.', minutes: 1 },
      ],
    },
  ],
  zh: [
    {
      name: '煎饼',
      steps: [
        { text: '将面粉、鸡蛋、牛奶和糖搅拌在一起。', minutes: 2 },
        { text: '让面糊静置。', minutes: 10 },
        { text: '加热平底锅，倒入一勺面糊。', minutes: 1 },
        { text: '将每个煎饼煎至金黄。', minutes: 3 },
      ],
    },
    {
      name: '番茄意面',
      steps: [
        { text: '将一大锅盐水煮沸。', minutes: 5 },
        { text: '将意面煮至弹牙。', minutes: 9 },
        { text: '用油煸炒大蒜和番茄。', minutes: 4 },
        { text: '沥干，与酱汁拌匀后上桌。', minutes: 1 },
      ],
    },
  ],
};

const STRINGS = {
  fr: {
    appTitle: 'Recette pas-à-pas',
    statusStarting: 'Démarrage…',
    statusConnected: 'Lunettes connectées',
    statusDisconnected: 'Lunettes déconnectées',
    start: 'Envoyer aux lunettes',
    stop: 'Arrêter',
    glassesHint: 'Sur les lunettes : tête haut/bas = changer d’étape, bouton = minuteur.',
    pickRecipe: 'Choisir une recette',
    stepTimer: (min) => (min > 0 ? `Étape de ${min} min` : 'Étape sans minuteur'),
    stepLine: (i, total) => `Étape ${i} / ${total}`,
    timerDone: (i) => `Minuteur de l’étape ${i} terminé !`,
    glassStep: (i) => `Sur les lunettes : étape ${i}`,
    connected: 'Recette envoyée aux lunettes.',
    statsRecipes: 'Recettes envoyées',
    noStats: 'Aucune recette envoyée pour l’instant.',
  },
  en: {
    appTitle: 'Step-by-step recipe',
    statusStarting: 'Starting…',
    statusConnected: 'Glasses connected',
    statusDisconnected: 'Glasses disconnected',
    start: 'Send to glasses',
    stop: 'Stop',
    glassesHint: 'On the glasses: head up/down = change step, button = timer.',
    pickRecipe: 'Pick a recipe',
    stepTimer: (min) => (min > 0 ? `${min} min step` : 'Step without a timer'),
    stepLine: (i, total) => `Step ${i} / ${total}`,
    timerDone: (i) => `Timer for step ${i} done!`,
    glassStep: (i) => `On the glasses: step ${i}`,
    connected: 'Recipe sent to the glasses.',
    statsRecipes: 'Recipes sent',
    noStats: 'No recipe sent yet.',
  },
  es: {
    appTitle: 'Receta paso a paso',
    statusStarting: 'Iniciando…',
    statusConnected: 'Gafas conectadas',
    statusDisconnected: 'Gafas desconectadas',
    start: 'Enviar a las gafas',
    stop: 'Detener',
    glassesHint: 'En las gafas: cabeza arriba/abajo = cambiar de paso, botón = temporizador.',
    pickRecipe: 'Elegir una receta',
    stepTimer: (min) => (min > 0 ? `Paso de ${min} min` : 'Paso sin temporizador'),
    stepLine: (i, total) => `Paso ${i} / ${total}`,
    timerDone: (i) => `¡Temporizador del paso ${i} terminado!`,
    glassStep: (i) => `En las gafas: paso ${i}`,
    connected: 'Receta enviada a las gafas.',
    statsRecipes: 'Recetas enviadas',
    noStats: 'Aún no se ha enviado ninguna receta.',
  },
  it: {
    appTitle: 'Ricetta passo passo',
    statusStarting: 'Avvio…',
    statusConnected: 'Occhiali connessi',
    statusDisconnected: 'Occhiali disconnessi',
    start: 'Invia agli occhiali',
    stop: 'Ferma',
    glassesHint: 'Sugli occhiali: testa su/giù = cambia passo, pulsante = timer.',
    pickRecipe: 'Scegli una ricetta',
    stepTimer: (min) => (min > 0 ? `Passo di ${min} min` : 'Passo senza timer'),
    stepLine: (i, total) => `Passo ${i} / ${total}`,
    timerDone: (i) => `Timer del passo ${i} terminato!`,
    glassStep: (i) => `Sugli occhiali: passo ${i}`,
    connected: 'Ricetta inviata agli occhiali.',
    statsRecipes: 'Ricette inviate',
    noStats: 'Nessuna ricetta inviata per ora.',
  },
  de: {
    appTitle: 'Schritt-für-Schritt-Rezept',
    statusStarting: 'Startet…',
    statusConnected: 'Brille verbunden',
    statusDisconnected: 'Brille getrennt',
    start: 'An die Brille senden',
    stop: 'Stoppen',
    glassesHint: 'Auf der Brille: Kopf hoch/runter = Schritt wechseln, Taste = Timer.',
    pickRecipe: 'Rezept wählen',
    stepTimer: (min) => (min > 0 ? `${min}-Min-Schritt` : 'Schritt ohne Timer'),
    stepLine: (i, total) => `Schritt ${i} / ${total}`,
    timerDone: (i) => `Timer für Schritt ${i} abgelaufen!`,
    glassStep: (i) => `Auf der Brille: Schritt ${i}`,
    connected: 'Rezept an die Brille gesendet.',
    statsRecipes: 'Gesendete Rezepte',
    noStats: 'Noch kein Rezept gesendet.',
  },
  zh: {
    appTitle: '分步食谱',
    statusStarting: '启动中…',
    statusConnected: '眼镜已连接',
    statusDisconnected: '眼镜未连接',
    start: '发送到眼镜',
    stop: '停止',
    glassesHint: '在眼镜上：抬头/低头 = 切换步骤，按钮 = 计时器。',
    pickRecipe: '选择食谱',
    stepTimer: (min) => (min > 0 ? `约 ${min} 分钟步骤` : '无计时步骤'),
    stepLine: (i, total) => `第 ${i} / ${total} 步`,
    timerDone: (i) => `第 ${i} 步计时结束！`,
    glassStep: (i) => `眼镜上：第 ${i} 步`,
    connected: '食谱已发送到眼镜。',
    statsRecipes: '已发送食谱',
    noStats: '暂无发送记录。',
  },
};

export function detectLocale() {
  const tag = (navigator.language || 'fr').toLowerCase();
  if (tag.startsWith('en')) return 'en';
  if (tag.startsWith('es')) return 'es';
  if (tag.startsWith('it')) return 'it';
  if (tag.startsWith('de')) return 'de';
  if (tag.startsWith('zh')) return 'zh';
  return 'fr';
}

export function getStrings(locale) {
  return STRINGS[locale] || STRINGS.fr;
}

export function getRecipes(locale) {
  return RECIPES[locale] || RECIPES.fr;
}
