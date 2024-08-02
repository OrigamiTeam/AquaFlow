O primeiro passo foi a realização da configuração dos registradores abaixo. Estes valores foram determinados através de testes e medições realizadas com o auxílio do osciloscópio.

No TDC1000 foram configurados os seguintes registradores:
- CONFIG_0 = 0x45
- CONFIG_1 = 0x48
- CONFIG_2 = 0x2A
- CONFIG_3 = 0x05
- TOF_1 = 0x00
- TOF_2 = 0x3F

No TDC7200 foram configurados os seguintes registradores:
- CONFIG2 = 0x48

Em seguida foram realizadas as medições seguindo a sequencia de acionamento abaixo, com atraso de 50ms entre cada operação:
1. Pino EN do TDC1000 para nível HIGH.
2. Pino CHSEL do TDC1000 para nível LOW (upstream).
3. Start da medição configurando o registrador CONFIG1 do TDC7200 com o valor 0x03.
4. Aguardar a finalização do ciclo de medição que é indicada pelo recebimento do nível LOW no pino INT do TDC7200.
5. Ler Tempo de Voo (ToF 1) no TDC7200 (upstream).
6. Pino CHSEL do TDC1000 para nível HIGH (downstream).
7. Start da medição configurando o registrador CONFIG1 do TDC7200 com o valor 0x03.
8. Aguardar a finalização do ciclo de medição que é indicada pelo recebimento do nível LOW no pino INT do TDC7200.
9. Ler tempo de Voo (ToF 1) no TDC7200 (downstream).
10. Calcular fluxo do meio com base na fórmula fornecida pelo fabricante no datasheet do TDC7200 onde:
    "ΔTOF" é a variação do tempo de voo (ToF upstream - ToF downstream) [ns]
    "c" é a velocidade do som na água e consideramos 1484 m/s = 1,484 mm/us
    "l" é a distância entre os refletores de ultrassom, uma vez que a distância entre o refletor e o transdutor foi desconsiderada. Esta distância foi desconsiderada pois a onda sonora está viajando perpendicular ao fluxo e não sofre interferência da velocidade do fluxo. [mm]
    "v" é a velocidade do fluxo [mm/ms]
11. Converter o fluxo calculado em vazão, considerando a secção transversal do tubo.
12. Ajustar o fluxo de acordo com o fator de correção "k".

O firmware que realiza estas medições pode ver verificado na pasta atual.

O fator de correção "k" foi determinado através da realização de medições utilizando um sensor de fluxo de efeito hall com aproximadamente 27 pulsos por litro. Foram realizadas medições em 5 fluxos diferentes, fazendo a média das leituras realizadas com o sensor de referência e através das medições do TDC1000 + TDC7200.

Em seguida, foi construído um gráfico de dispersão tendo como eixo X os valores de fluxo medido e como eixo Y os valores de "k" calculados para cada um dos 5 ensaios realizados.

Por meio do gráfico de dispersão foi possível gerar uma linha de tendência que indica o valor do "k" que deve ser utilizado de acordo com o fluxo medido e, com isso, foi possível realizar a aproximação do fluxo medido ao fluxo de referência do sensor de fluxo de efeito hall.

Planilha com os cálculos citados anteriormente: https://docs.google.com/spreadsheets/d/1ap94_Y3gvMaDul2QDZ6mnZ87WAkQDQjPwV1WzfmNDjM


A diferença máxima entre as medições de fluxo foi por volta de 1 L/min para o fluxo de 25 L/min, ou seja, estamos trabalhando com uma diferença máxima de 4% quando comparados os medidores.
