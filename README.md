# ENTER FELLOWSHIP: Take Home Project

Ferramenta C++ que:

1. le um dataset e os fields pedidos em JSON;
2. le o pdf relacionado ao dataset;
3. extrai e outputa os fields pedidos por parsing local e chamada a API do gpt-5-mini;

O resultado final é gravado em `outputs.json`.

## Dependencias

* CMake ≥ 3.20 e compilador C++20
* pkg-config
* **Poppler C++** (`poppler-cpp`)
* **libcurl**
* **git** (para buscar simdjson e nlohmann/json)

### Instalacao rapida

**Ubuntu/Debian**

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config \
  libpoppler-cpp-dev libcurl4-openssl-dev git
```

**macOS (Homebrew)**

```bash
brew install cmake pkg-config poppler curl git
```

## Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Gera o executavel **`build/enter`**.

## Execução

1. Exporte sua chave da OpenAI:

```bash
export OPENAI_API_KEY="sk-...sua-chave..."
```

2. Rodar:

```bash
./build/enter <caminho_dataset.json> <pasta_dos_pdfs>
```

Exemplo:

```bash
./build/enter data/dataset.json data/pdfs
```

## Ideias usadas

## 1. MVP: escolhas e pipeline

**Poppler**
Usei a API C++ do Poppler para duas leituras complementares:

* **Texto integral da pagina** (`page->text(page_rect())`), que alimenta o prompt do modelo;
* **Caixas de texto** (`page->text_list()`), onde cada `text_box` traz string + retangulo (bbox).
  Fiz uma simples ordenacao por `(página, y0, x0)` e um **merge linear** de caixas proximas (mesma linha por tolerância de centro-Y e gap-X pequeno). Esse passo eh barato ( (O(N\log N)) pela ordenacao + (O(N)) varredura) e deixa os trechos mais “legíveis” para parsings locais (vai fazer mais sentido na parte da otimizacao que pensei).

**nlohmann::json (estrutura e ergonomia).**
O dataset entra como um array de objetos com `label`, `extraction_schema` e `pdf_path`. Leitura rapida com **simdjson** (via `ondemand`) e convercao para `nlohmann::json` apenas onde precisa manipular.

**libcurl + prompt engineering simples.**
A chamada HTTP é feita com **libcurl**, mandando `conteudo` (montado por `api::formata_nlohmann`) para o endpoint da OpenAI. O **prompt** eh propositalmente minimalista e **determinístico**. Infelizmente o modelo escolhido nao deixa setar temperature entao dependi do prompt pra tentar manter uma acuracia decente.

---

## 2. Otimização: edit distance de prefixos (extensao de aprendizado de maratona)

**Objetivo**
Lidar com dados que aparecem no pdf como: {chave}: {valor}, ou {chave} - {valor} de maneira mais eficiente. Me pareceu como util uma vez que formatos como esse sao bem comuns na formatacao de documentos.

**O que foi adicionado.**
Antes de chamar o modelo, para **cada campo** do `extraction_schema` (o “padrão” eh a **descrição** do campo), o programa varre todas as `text_box` do PDF e calcula a **edit distance de A para cada prefixo de B**. Eh basicamente a quantidade de tecladas que distancia A de B (inserir, remover ou trocar char em string).

* (A): a **descricao** do campo (ex.: “Nome do profissional…”);
* (B): o **texto** da caixa;
* guarda ((\text{dist}, \text{pos})) do melhor prefixo (menor edit distance).
  Se `dist < LIM` (padrão `LIM=3`, porque parecia um limite decente pra mim), pega o **sufixo** `B[pos..]` como **candidato** para aquele campo e anexa a descricao:
  `possiveis respostas: cand1 | cand2 | ...`

Na pratica, isso ajuda o modelo na decisao e prospeccao de possiveis respostas.

**Como computar isso rapido.**
Usamos classica dp quadratica da **Edit distance**, mas com **apenas duas linhas** (fila/atual), atualizando por caractere de (B).
Complexidade:

* **Tempo**: (O(mn)) para saber a distância de (A) para **todo prefixo** de (B), onde (m=|A|, n=|B|).
  Isso é exatamente o que queremos, pois o valor na última célula da linha (i) é a distância até o prefixo (B[0..i)).
* **Memória**: (O(m)).


**Implementacao.**

* Funcao `opt::min_pref_editdistance(A, B)` devolve `(dist, pos)` do melhor prefixo;
* No `main`, para cada campo, varre as caixas; se `dist < LIM`, anexa `B.substr(pos)` como candidato;
* Mantemos no max 5 candidatos distintos por campo para nao poluir demais o prompt.

---

## 3. Algumas outras possiveis melhoras.

* **Outros Formatos pro edit distance**: eh possivel (e parece comum) que {chave} {valor} aparecam um encima do outro. Meu codigo nao consegue pegar esses casos, e nao adicionei por falta de tempo.
* **Ukkonen**: esse edit distance de prefixo nao eh exatamente perfeito com o problema. se so nos importa “<= k erros”, eh possivel usar esse algoritmo que li faz pouco tempo pra alcancar
  **(O(nk))** tempo, **(O(m))** memória. Pareceu overkill, considerando que o pattern (chave) no nosso casoe eh usualmente pequeno.
* **Regex**: alguns campos tem formato conhecido, como CPF, CNPJ, telefones... Eh possivel achar substrings do pdf em que campos como esse aparecem e usalos como candidatos. Me pareceu mais trabalhoso
  que criativo por isso nao segui. Mas com certeza seria promissor.
