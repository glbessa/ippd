A Tarefa D, denominada **Organização de região paralela**, exige a implementação e comparação de duas variantes diferentes para executar múltiplos laços paralelos. O foco é avaliar o *overhead* (custo adicional) associado à criação e finalização de regiões paralelas.

As duas variantes que devem ser implementadas são:

1.  **Variante Ingênua:** Consiste na implementação de **dois `parallel for` consecutivos**.
2.  **Variante Arrumada:** Consiste na implementação de **uma única região `parallel`** que englobe os dois `for` internos. Esta abordagem é recomendada para evitar o paralelismo aninhado desnecessário, conforme o objetivo geral das atividades.

O objetivo principal desta tarefa é **comparar o *overhead* e os tempos de execução** dessas duas abordagens.

Em termos de organização de código, a diretriz geral de implementação paralela, que se aplica a esta tarefa, é usar uma **única região `parallel`** que englobe os laços `for` internos, sempre que houver dois laços paralelos em sequência.