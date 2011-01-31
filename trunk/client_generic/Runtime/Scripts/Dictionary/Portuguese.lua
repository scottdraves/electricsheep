module( ... )
messages = {

-- About tab.
aboutDesc = "Sobre",

-- Display tab. 
displayTab = "Display",
displayDesc = "Estas definições afetam a apresentação visual dos ovelhas na tela",
Fullscreen = "Apresentação em tela cheia (não há necessidade de mudar esta exceto para fins de depuração)",
Iterations = "O número de vezes para exibir uma looping ovelha",
Piecewise = "Apresentação linear por partes. Produz reprodução suave por interpolação entre quadros.",
Fps = "O número de vezes por segundo a decodificar um novo quadro a partir da ovelha.\nDiminua esse valor se a sua máquina não é rápida o suficiente",
BufferLen = "O número de quadros armazenados com antecedência.\nDiminua esse valor se memória RAM for um problema.",

-- Server tab. 
serverTab = "Servidor",
serverDesc = "Essas configurações definem a forma de comunicar com o servidor de ovelhas",
ServerUrl = "O servidor de ovelhas",
SheepDir = "Localização de seus cache de ovelhas",
CacheSize = "A quantidade de espaço em disco que as ovelhas podem consumir",
DownloadMode = "Permite o download de novas ovelhas",
UseTorrent = "Usar BitTorrent peer-to-peer download (desativada em versão beta)",
Registered = "Você é um usuário registrado?",
Password = "Sua senha",
UniqueID = "O seu ID de cliente",
UseProxy = "Usar um servidor proxy?",
Proxy = "endereço IP do servidor proxy",
Proxy_User = "nome de usuário no servidor proxy",
Proxy_Pass = "senha no servidor proxy",

-- Renderer tab. 
rendererTab = "Renderizador",
rendererDesc = "Estas definições afetam como o seu cliente apresenta novas ovelhas",
DemoMode = "Ativar renderização de quadros de ovelhas",
AllCores = "Utilizar todos os núcleos do processador para renderizar ovelhas",
SaveFrames = "Salvar quadros concluídos",
Nickname = "Seu nickname",
UserUrl = "Link para a sua página web inicial",

-- Logs tab. 
logsTab = "Logs",
logsDesc = "Logging informação",

-- Statistics tab.
statsTab = "Estatísticas",
statsDesc = "Suas estatísticas sobre ovelhas"
}