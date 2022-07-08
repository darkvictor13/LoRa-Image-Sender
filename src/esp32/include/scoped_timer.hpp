/**
 * @file scoped_timer.h
 * @author Victor Emanuel Almeida (victoralmeida2001@hotmail.com)
 * @brief Arquivo que define um marcador de tempo de execução.
 * 
 * Quando o objeto definido pela classe ScopedTimer é criado,
 * ele começa a contar a quantidade de tempo. Quando o objeto sair de
 * escopo seu destrutor é chamado marcando assim o tempo em segundos
 * @version 1.0
 * @date 10/02/2022
 */

#ifndef SCOPED_TIMER
#define SCOPED_TIMER

#include <Arduino.h>

/**
 * @brief Cria um Timer no estilo RAII
 * 
 * Quando construído salva o momento que foi chamado,
 * ao ser destruído faz log do tempo de execução
 * @pre ser chamado em um escopo não global
 * @post Nenhuma
 */
#define SET_TIMER(name) ws::ScopedTimer _time(name)
/**
 * @brief Cria um timer com o nome da função que está em escopo
 * 
 * @pre ser chamado em um escopo não global
 * @post Nenhuma
 */
#define SET_TIMER_DEFAULT//ws::ScopedTimer _time(__func__)

namespace ws {
	/**
	 * @brief Utilizando o princípio RAII, cria um timer que conta o tempo de execução
	 */
	class ScopedTimer {
	private:
		const char *scope_name;
		unsigned long start;
	public:
		/**
		 * @brief Construtor da classe Scoped Timer
		 * 
		 * @param name O nome do escopo
		 * @pre Nenhuma
		 * @post Nenhuma
		 */
		ScopedTimer(const char *name);

		/**
		 * @brief Obtêm o tempo de execução em segundos
		 * 
		 * @return double o tempo de execução em segundos
		 * @pre Nenhuma
		 * @post Nenhuma
		 */
		double getExecutionTimeInSeconds() const;

		/**
		 * @brief Obtêm o tempo de execução em nanosegundos
		 * 
		 * @return uint64_t o tempo de execução em nanosegundos
		 * @pre Nenhuma
		 * @post Nenhuma
		 */
		int64_t getExecutionTimeInUs() const;

		/**
		 * @brief Destrutor da classe Scoped Timer
		 * 
		 * @pre Nenhuma
		 * @post Imprime no serial o tempo de execução do escopo
		 */
		~ScopedTimer();
	};
}

#endif // SCOPED_TIMER
