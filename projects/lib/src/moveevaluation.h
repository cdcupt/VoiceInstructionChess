/*
    This file is part of Cute Chess.

    Cute Chess is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Cute Chess is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Cute Chess.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MOVEEVALUATION_H
#define MOVEEVALUATION_H

#include <QString>
#include <QMetaType>

/*!
 * \brief Evaluation data for a chess move.
 *
 * Before chess engines send their move they usually print information
 * about the moves they're thinking of, how many nodes they've searched,
 * the search depth, etc. This class stores that information so that it
 * could be saved in a PGN file or displayed on the screen.
 *
 * From human players we can only get the move time.
 */
class LIB_EXPORT MoveEvaluation
{
	public:
		/*! A value for a null or empty score. */
		static const int NULL_SCORE = 0xFFFFFFF;

		/*! Constructs an empty MoveEvaluation object. */
		MoveEvaluation();

		/*! Returns true if \a other is the same as this eval. */
		bool operator==(const MoveEvaluation& other) const;
		/*! Returns true if \a other is different from this eval. */
		bool operator!=(const MoveEvaluation& other) const;

		/*! Returns true if the evaluation is empty. */
		bool isEmpty() const;
		
		/*! Returns true if the evaluation points to a book move. */
		bool isBookEval() const;

		/*!
		 * How many plies were searched?
		 * \note For human players this is always 0.
		 */
		int depth() const;

		/*!
		 * Score in centipawns from the player's point of view.
		 * \note For human player this always 0.
		 */
		int score() const;

		/*! Move time in milliseconds. */
		int time() const;

		/*!
		 * How many nodes were searched?
		 * \note For human players this is always 0.
		 */
		quint64 nodeCount() const;

		/*!
		 * The principal variation.
		 * This is a sequence of moves that an engine
		 * expects to be played next.
		 * \note For human players this is always empty.
		 */
		QString pv() const;


		/*! Resets everything to zero. */
		void clear();

		/*! Sets book evaluation. */
		void setBookEval(bool isBookEval);

		/*! Sets the search depth to \a depth. */
		void setDepth(int depth);

		/*! Sets the score to \a score. */
		void setScore(int score);

		/*! Sets the move time to \a time. */
		void setTime(int time);

		/*! Sets the node count to \a nodeCount. */
		void setNodeCount(quint64 nodeCount);

		/*! Sets the principal variation to \a pv. */
		void setPv(const QString& pv);

		/*! Merges non-empty parameters of \a other into this eval. */
		void merge(const MoveEvaluation& other);

	private:
		bool m_isBookEval;
		int m_depth;
		int m_score;
		int m_time;
		quint64 m_nodeCount;
		QString m_pv;
};

Q_DECLARE_METATYPE(MoveEvaluation)

#endif // MOVEEVALUATION_H
